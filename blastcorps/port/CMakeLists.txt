# The Windows port built with MSVC (x64): bc_headless.exe and bc.exe.
# See README.md, "Build on Windows with Visual Studio".
#
#   cmake --preset x64-release && cmake --build --preset x64-release
#
# Prerequisites: Visual Studio 2022 or later with "Desktop development with
# C++" (MSVC x64, the Windows SDK, CMake, Ninja), Python 3 (standard library
# only), and the inputs folder from the decomp side (BC_INPUTS below).
#
# How the build works (the same steps as port/Makefile and port64.mk, on
# Windows):
#   - the game C (src.us.v11/hd_code, hd_front_end), the ultralib objects and
#     the platform files that see <ultra64.h> are compiled with the game's
#     headers (/X: no C runtime headers) into object libraries;
#   - tools/coffpin.py rewrites those objects: every data object at an N64
#     address keeps its bytes as __native_<name> and its users go to the
#     address itself (an absolute symbol from tools/gensyms.py linkcoff);
#   - generated tables: the copy of the native initialisers (copytab), the
#     pointer slots of N64 data (ptrtab), the byte order of the data images
#     (swaptab), traps for undefined functions (stubs);
#   - the exe is linked at a fixed base (0x40000000) without ASLR, so N64
#     memory at 0x80000000 is reachable RIP-relative and every host address the
#     game stores is below 4 GB.
cmake_minimum_required(VERSION 3.25)
project(BlastCorpsPort LANGUAGES C CXX ASM_MASM)

if(NOT MSVC OR NOT CMAKE_C_COMPILER_ID STREQUAL "MSVC" OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project builds with MSVC for x64 only (Visual Studio's \"x64\" environment).")
endif()

# ---- locations and inputs ------------------------------------------------------
set(BC_PORT ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(BC_ROOT "${BC_PORT}/.." ABSOLUTE)
set(BC_HD_SRC ${BC_ROOT}/src.us.v11/hd_code)
set(BC_FE_SRC ${BC_ROOT}/src.us.v11/hd_front_end)
set(BC_UL_DIR "${BC_ROOT}/../lib/ultralib" CACHE PATH "ultralib (the lib/ultralib submodule)")
# What the build takes from the NON_MATCHING (IDO) build of the decomp: data
# addresses, symbol kinds, the N64 pointer slots, the C declarations' types and
# the NM link maps.  Made in WSL by `make -C port inputs` (port/build/inputs).
set(BC_INPUTS "${BC_PORT}/build/inputs" CACHE PATH "the decomp inputs folder (make -C port inputs)")
set(BC_THIRDPARTY "${CMAKE_BINARY_DIR}/thirdparty" CACHE PATH "downloads (SDL2, DXC, RT64)")
option(BC_GAME "also build bc.exe (RT64, SDL2, DXC are downloaded)" ON)

foreach(f addrs.txt elfsyms.txt nmptrs.txt typemap.txt hd_code.us.v11.map hd_front_end.us.v11.map)
    if(NOT EXISTS "${BC_INPUTS}/${f}")
        message(FATAL_ERROR "BC_INPUTS (${BC_INPUTS}) has no ${f}.\n"
            "Make the inputs folder in WSL: `make -C port inputs` (it needs the NON_MATCHING build of the "
            "decomp), then point BC_INPUTS at it (e.g. \\\\wsl.localhost\\Ubuntu\\home\\you\\blastcorps\\"
            "blastcorps\\port\\build\\inputs) or copy it to port/build/inputs.")
    endif()
endforeach()
if(NOT EXISTS "${BC_UL_DIR}/include/PR/ultratypes.h")
    message(FATAL_ERROR "BC_UL_DIR (${BC_UL_DIR}) is not ultralib: run `git submodule update --init lib/ultralib`.")
endif()

find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
set(PY "${Python3_EXECUTABLE}")
set(TOOLS ${BC_PORT}/tools)
set(GEN ${CMAKE_BINARY_DIR}/gen)
set(BC_IMAGE_BASE 0x40000000)
file(MAKE_DIRECTORY ${GEN})

# ---- the version (git hash) the exes report ---------------------------------------
set(BC_GIT_VERSION "" CACHE STRING "the version the exes report (default: git's short hash of the sources)")
add_custom_target(bc_version
    COMMAND ${CMAKE_COMMAND} -DOUT=${GEN}/version.h -DSRC=${BC_PORT} -DOVERRIDE=${BC_GIT_VERSION}
        -P ${BC_PORT}/cmake/version.cmake
    BYPRODUCTS ${GEN}/version.h)

# ---- source lists (as port/Makefile) ----------------------------------------------
# hd_code files that are only SDK asm or RCP-register C: the platform replaces them
set(HL_HD_SDK 8FD90 906E0 91820 91F50 91F80 92EA0 95060 96300 96F70 96FF0 971D0 971F0 972A0
    97B70 98330 9DB70 9DB80 9E6F0 A0A30)
file(GLOB _hd RELATIVE ${BC_HD_SRC} CONFIGURE_DEPENDS ${BC_HD_SRC}/*.c)
file(GLOB _fe RELATIVE ${BC_FE_SRC} CONFIGURE_DEPENDS ${BC_FE_SRC}/*.c)
set(HD_SRCS)
foreach(f ${_hd})
    get_filename_component(b ${f} NAME_WE)
    if(NOT b MATCHES "^ul_" AND NOT b IN_LIST HL_HD_SDK)
        list(APPEND HD_SRCS ${BC_HD_SRC}/${f})
    endif()
endforeach()
set(FE_SRCS)
foreach(f ${_fe})
    if(NOT f MATCHES "^ul_")
        list(APPEND FE_SRCS ${BC_FE_SRC}/${f})
    endif()
endforeach()
# the callers of osGetTime/osGetCount keep their own functions (--clock)
set(HL_CLOCK_CALLERS 00000 1C460 20460 22EE0 23C20 2C560)
# ultralib objects compiled natively (gu, libm, printf, libaudio)
set(HL_UL ul_lookat ul_lookatref ul_mtxcatf ul_mtxcatl ul_mtxutil ul_normalize ul_ortho
    ul_perspective ul_rotate ul_scale ul_translate ul_sins ul_coss
    ul_sprintf ul_xlitob ul_ldiv
    ul_auxbus ul_bnkf ul_cents2ratio ul_copy ul_cseq ul_cspgetstate ul_cspgettempo ul_csplayer
    ul_cspplay ul_cspsetbank ul_cspsetseq ul_cspsettempo ul_cspsetvol ul_cspstop ul_drvrnew
    ul_env ul_event ul_filter ul_heapalloc ul_heapinit ul_load ul_mainbus ul_resample ul_reverb
    ul_save ul_seq ul_seqplayer ul_sl ul_synaddplayer ul_synallocfx ul_synallocvoice
    ul_synfreevoice ul_synsetfxmix ul_synsetpan ul_synsetpitch ul_synsetpriority ul_synsetvol
    ul_synstartvoice ul_synstartvoiceparam ul_synstopvoice ul_synthesizer)
# the front end's own SDK objects (Controller Pak file system over pif.c)
set(HL_ULFE ul_crc ul_contpfs ul_contramread ul_contramwrite ul_pfsallocatefile ul_pfschecker
    ul_pfsdeletefile ul_pfsfilestate ul_pfsfreeblocks ul_pfsinitpak ul_pfsisplug ul_pfsreadwritefile
    ul_pfssearchfile)
list(REMOVE_ITEM HL_ULFE ul_pfsallocatefile)   # (from le/pfsallocatefile.c, below)
set(HL_PLAT os_thread os_time os_hw os_si input plat_core save pif)

# ---- generated sources: ultralib with N64 pointer marks, little-endian edits -----
set(UL_WRAPPERS)
set(UL_GEN)
foreach(n ${HL_UL})
    list(APPEND UL_WRAPPERS ${BC_HD_SRC}/${n}.c)
    list(APPEND UL_GEN ${GEN}/ulsrc/${n}.c)
endforeach()
set(ULFE_GEN)
foreach(n ${HL_ULFE})
    list(APPEND UL_WRAPPERS ${BC_FE_SRC}/${n}.c)
    list(APPEND ULFE_GEN ${GEN}/ulsrc/${n}.c)
endforeach()
string(REPLACE ";" "\n" _w "${UL_WRAPPERS}")
file(WRITE ${GEN}/ul_wrappers.txt "${_w}\n")
add_custom_command(OUTPUT ${GEN}/ulinc.stamp ${UL_GEN} ${ULFE_GEN}
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${GEN}/ulinc ${GEN}/ulsrc
    COMMAND ${PY} ${TOOLS}/ulhdr64.py ${BC_UL_DIR} ${BC_PORT}/data/ultralib_n64ptr.txt ${GEN}/ulinc
    COMMAND ${PY} ${TOOLS}/ulsrc64.py ${BC_UL_DIR} ${GEN}/ulinc ${GEN}/ulsrc @${GEN}/ul_wrappers.txt
    COMMAND ${CMAKE_COMMAND} -E touch ${GEN}/ulinc.stamp
    DEPENDS ${TOOLS}/ulhdr64.py ${TOOLS}/ulsrc64.py ${BC_PORT}/data/ultralib_n64ptr.txt ${UL_WRAPPERS}
    COMMENT "ultralib with N64 pointer marks (ulhdr64.py, ulsrc64.py)")
function(lesrc kind in out)
    add_custom_command(OUTPUT ${out} COMMAND ${PY} ${TOOLS}/lesrc.py ${kind} ${in} ${out}
        DEPENDS ${in} ${TOOLS}/lesrc.py COMMENT "little-endian copy: ${out}")
endfunction()
lesrc(xldtob ${BC_UL_DIR}/src/libc/xldtob.c ${GEN}/le/xldtob.c)
lesrc(xprintf ${BC_HD_SRC}/ul_xprintf.c ${GEN}/le/xprintf.c)
lesrc(gudouble ${BC_UL_DIR}/src/gu/sinf.c ${GEN}/le/sinf.c)
lesrc(gudouble ${BC_UL_DIR}/src/gu/cosf.c ${GEN}/le/cosf.c)
lesrc(controller ${BC_UL_DIR}/include/PRinternal/controller.h ${GEN}/le/PRinternal/controller.h)
lesrc(pfsalloc ${BC_UL_DIR}/src/io/pfsallocatefile.c ${GEN}/le/pfsallocatefile.c)

# ---- compiler flags -------------------------------------------------------------
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# a PDB for everything (crash reports, the debugger)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")
string(REGEX REPLACE "/O[12xd]|/Ob[0-3]|/RTC1|/DNDEBUG" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
string(REGEX REPLACE "/O[12xd]|/Ob[0-3]|/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(BC_OPT $<IF:$<CONFIG:Debug>,/Od,/O2>)

# Game code (and the platform files that see <ultra64.h>): N64 layouts and the
# game's own headers.
#   /X                    no C runtime headers (as -nostdinc)
#   /Gw /Gy               one section per variable and function (coffpin.py)
#   /GS-                  no stack cookies
#   /Ob1 (game files)     only `inline` functions are inlined: --sync and --clock
#                         key the emulator's values on the calling game function
#   /GH (game files)      a _pexit call (empty, entry_x64.asm) before every return:
#                         keeps MSVC from turning calls in tail position into jumps,
#                         so a return address names its real caller (as gcc's
#                         -fno-optimize-sibling-calls)
#   /hotpatch (game files) + /FUNCTIONPADMIN (link): a first instruction of 2+ bytes
#                         and free bytes before every function, where plat_host.c
#                         patches in --sync's function-entry switch points and
#                         --calls (the role of -finstrument-functions)
#   no -fwrapv, -fno-strict-aliasing, -malign-double, -mno-ms-bitfields: MSVC does
#   no type-based alias analysis, aligns 8-byte members naturally, and the GBI's
#   bitfields are written with one storage type (checked: layout_check.c)
set(GAME_INC ${BC_ROOT} ${BC_ROOT}/include ${BC_ROOT}/include/2.0I ${BC_ROOT}/include/2.0I/PR)
set(GAME_DEFS _LANGUAGE_C _FINALROM NON_MATCHING PORT_HOST _MIPS_SZLONG=32 _MIPS_SIM=1)
set(GAME_OPTS /X /std:c17 /W1 /wd4311 /wd4312 /wd4068 /GS- /Gw /Gy ${BC_OPT}
    /FI${BC_PORT}/include/port_ultratypes.h)
set(UL_INC ${GEN}/ulinc ${BC_UL_DIR} ${GEN}/ulinc/include ${BC_PORT}/include/msvc
    ${GEN}/ulinc/include/compiler/modern_gcc ${GEN}/ulinc/include/PR ${GEN}/ulinc/src/libc)
# (_SIZE_T_DEF: MSVC predefines size_t, which ultralib's headers would redefine
# as a 32-bit type)
set(UL_DEFS _MIPS_SZLONG=32 BUILD_VERSION=VERSION_I "BUILD_VERSION_STRING=\"2.0I\"" NDEBUG _FINALROM F3DEX_GBI
    _LANGUAGE_C _SIZE_T_DEF)
set(UL_OPTS /X /std:c17 /w /GS- /Gw /Gy /Oi- ${BC_OPT} /FI${BC_PORT}/include/port_n64ptr.h
    /FI${BC_PORT}/include/msvc/ul_msvc.h)

function(game_lib name)
    cmake_parse_arguments(A "" "" "SOURCES;INC;DEFS;OPTS;DEPS" ${ARGN})
    add_library(${name} OBJECT ${A_SOURCES})
    target_include_directories(${name} PRIVATE ${A_INC})
    target_compile_definitions(${name} PRIVATE ${A_DEFS})
    target_compile_options(${name} PRIVATE ${A_OPTS})
    if(A_DEPS)
        add_dependencies(${name} ${A_DEPS})
    endif()
endfunction()

add_custom_target(bc_ulgen DEPENDS ${GEN}/ulinc.stamp ${GEN}/le/PRinternal/controller.h)
game_lib(bc_hd SOURCES ${HD_SRCS} INC ${GAME_INC} DEFS ${GAME_DEFS}
    OPTS ${GAME_OPTS} $<$<NOT:$<CONFIG:Debug>>:/Ob1> /GH /hotpatch)
game_lib(bc_fe SOURCES ${FE_SRCS} INC ${GAME_INC} DEFS ${GAME_DEFS}
    OPTS ${GAME_OPTS} $<$<NOT:$<CONFIG:Debug>>:/Ob1> /GH /hotpatch)
foreach(n ${HL_CLOCK_CALLERS})
    set_source_files_properties(${BC_HD_SRC}/${n}.c PROPERTIES COMPILE_OPTIONS /Ob0)
endforeach()
game_lib(bc_ul SOURCES ${UL_GEN} ${GEN}/le/xldtob.c ${GEN}/le/xprintf.c ${GEN}/le/sinf.c ${GEN}/le/cosf.c
    INC ${UL_INC} ${BC_UL_DIR}/src/gu DEFS ${UL_DEFS} OPTS ${UL_OPTS} DEPS bc_ulgen)
game_lib(bc_ulfe SOURCES ${ULFE_GEN} ${GEN}/le/pfsallocatefile.c
    INC ${GEN}/le ${UL_INC} DEFS ${UL_DEFS} OPTS ${UL_OPTS} DEPS bc_ulgen)
set(PLAT_SRCS)
foreach(n ${HL_PLAT})
    list(APPEND PLAT_SRCS ${BC_PORT}/src/platform/${n}.c)
endforeach()
game_lib(bc_plat SOURCES ${PLAT_SRCS} ${BC_PORT}/src/platform/layout_check.c
    INC ${GAME_INC} ${BC_PORT}/src/platform DEFS ${GAME_DEFS} OPTS ${GAME_OPTS})

# loadcheck (the load layer against the original code, `make -C port loadcheck-msvc`):
# its game files on their own, without the platform's hooks
set(LC_FILES 00000 46C20 17A70 53220 7F8B0 5BF40 60F60 5CB60 60D50)
set(LC_SRCS)
foreach(n ${LC_FILES})
    list(APPEND LC_SRCS ${BC_HD_SRC}/${n}.c)
endforeach()
game_lib(bc_lc SOURCES ${LC_SRCS} INC ${GAME_INC} DEFS ${GAME_DEFS} OPTS ${GAME_OPTS})

# ---- pinning: coffpin.py over each object library's objects -----------------------
# ${lib}_PINNED / ${lib}_RECS: the pinned objects and their records
set(PIN ${CMAKE_BINARY_DIR}/pin/$<CONFIG>)
function(pin_lib lib)
    get_target_property(srcs ${lib} SOURCES)
    set(objs)
    set(recs)
    foreach(s ${srcs})
        get_filename_component(stem ${s} NAME_WE)
        list(APPEND objs ${PIN}/${lib}/${stem}.obj)
        list(APPEND recs ${PIN}/${lib}/${stem}.rec)
    endforeach()
    file(GENERATE OUTPUT ${PIN}/${lib}.objs.txt CONTENT "$<JOIN:$<TARGET_OBJECTS:${lib}>,\n>\n")
    add_custom_command(OUTPUT ${objs} ${recs}
        COMMAND ${PY} ${TOOLS}/coffpin.py --base ${BC_IMAGE_BASE} --elfsyms ${BC_INPUTS}/elfsyms.txt
            --batch ${BC_INPUTS}/addrs.txt ${PIN}/${lib} ${PIN}/${lib}.objs.txt
        DEPENDS ${lib} $<TARGET_OBJECTS:${lib}> ${TOOLS}/coffpin.py ${BC_INPUTS}/addrs.txt
            ${BC_INPUTS}/elfsyms.txt
        COMMENT "pinning ${lib}'s N64 data (coffpin.py)" VERBATIM)
    set(${lib}_PINNED ${objs} PARENT_SCOPE)
    set(${lib}_RECS ${recs} PARENT_SCOPE)
endfunction()
foreach(l bc_hd bc_fe bc_ul bc_ulfe bc_plat bc_lc)
    pin_lib(${l})
endforeach()

# ---- host code -------------------------------------------------------------------
set(HOST_INC ${BC_PORT}/src ${BC_PORT}/src/platform ${BC_PORT}/src/load)
set(HOST_OPTS /W3 /wd4996 /wd4244 /wd4267 /wd4311 /wd4312 /GS- /Gy ${BC_OPT})
set(HOST_DEFS _CRT_SECURE_NO_WARNINGS)
function(host_lib name)
    add_library(${name} OBJECT ${ARGN})
    target_include_directories(${name} PRIVATE ${HOST_INC} ${GEN})
    target_compile_definitions(${name} PRIVATE ${HOST_DEFS})
    target_compile_options(${name} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:${HOST_OPTS}>)
endfunction()
# the byte-order table of the data images (swaptab.py)
add_custom_command(OUTPUT ${GEN}/swaptab.c
    COMMAND ${PY} ${TOOLS}/swaptab.py ${BC_INPUTS}/typemap.txt ${BC_PORT}/data/image_widths.txt ${GEN}/swaptab.c
        ${GEN}/swaptab_report.txt --static ${BC_PORT}/data/image_static.txt ${BC_INPUTS}/addrs.txt
        --maps ${BC_INPUTS}/hd_code.us.v11.map ${BC_INPUTS}/hd_front_end.us.v11.map
    DEPENDS ${TOOLS}/swaptab.py ${BC_INPUTS}/typemap.txt ${BC_PORT}/data/image_widths.txt
        ${BC_PORT}/data/image_static.txt ${BC_PORT}/data/image_overrides.txt ${BC_INPUTS}/addrs.txt
    COMMENT "byte-order table of the data images (swaptab.py)" VERBATIM)
# RDRAM, the ROM and the load layer (byte order)
host_lib(bc_core ${BC_PORT}/src/rdram.c ${BC_PORT}/src/inflate.c ${BC_PORT}/src/load/swap.c
    ${BC_PORT}/src/load/port_assets.c ${BC_PORT}/src/load/gfx_fix.c ${GEN}/swaptab.c)
# the platform's host side: fibers, logging, crash reports, the entry hook, audio
host_lib(bc_host ${BC_PORT}/src/platform/plat_host.c ${BC_PORT}/src/platform/crash.c
    ${BC_PORT}/src/platform/entry_x64.asm ${BC_PORT}/src/audio/aspmain.c ${BC_PORT}/src/audio/port_audio.c)
host_lib(bc_headless_main ${BC_PORT}/src/platform/headless_main.c)
host_lib(bc_lc_main ${BC_PORT}/src/loadcheck_main.c)

# ---- link-time tables, per exe -----------------------------------------------------
# link_tables(NAME LIBS pinned game libraries.. HOST other object libraries..):
#   NAME_tables (object library: stubs.c, copytab.c, ptrtab.c) and NAME_LINK (the
#   pinned objects and abs_syms.obj, for the exe's sources)
function(link_tables name)
    cmake_parse_arguments(A "" "" "LIBS;HOST" ${ARGN})
    set(lt ${CMAKE_BINARY_DIR}/link/$<CONFIG>/${name})
    set(pinned)
    set(recs)
    foreach(l ${A_LIBS})
        list(APPEND pinned ${${l}_PINNED})
        list(APPEND recs ${${l}_RECS})
    endforeach()
    string(REPLACE ";" "\n" _p "${pinned}")
    file(GENERATE OUTPUT ${lt}/pinned.txt CONTENT "${_p}\n")
    string(REPLACE ";" "\n" _r "${recs}")
    file(GENERATE OUTPUT ${lt}/recs.txt CONTENT "${_r}\n")
    set(hostobjs)
    set(hostdeps)
    foreach(h ${A_HOST})
        string(APPEND hostobjs "$<JOIN:$<TARGET_OBJECTS:${h}>,\n>\n")
        list(APPEND hostdeps ${h} $<TARGET_OBJECTS:${h}>)
    endforeach()
    file(GENERATE OUTPUT ${lt}/host.txt CONTENT "${hostobjs}")
    add_custom_command(OUTPUT ${lt}/abs_syms.obj ${lt}/stubs.c ${lt}/link_report.txt
        COMMAND ${PY} ${TOOLS}/gensyms.py linkcoff ${lt} ${BC_INPUTS}/elfsyms.txt ${BC_IMAGE_BASE}
            --game ${lt}/pinned.txt --host ${lt}/host.txt
        DEPENDS ${pinned} ${hostdeps} ${TOOLS}/gensyms.py ${TOOLS}/rdramobj.py ${TOOLS}/coffpin.py
            ${BC_INPUTS}/elfsyms.txt
        COMMENT "${name}: absolute N64 symbols and stubs (gensyms.py linkcoff)" VERBATIM)
    add_custom_command(OUTPUT ${lt}/copytab.c
        COMMAND ${PY} ${TOOLS}/gensyms.py copytab64 ${lt}/copytab.c ${BC_INPUTS}/addrs.txt @${lt}/recs.txt
        DEPENDS ${recs} ${TOOLS}/gensyms.py COMMENT "${name}: copy table (gensyms.py copytab64)" VERBATIM)
    add_custom_command(OUTPUT ${lt}/ptrtab.c
        COMMAND ${PY} ${TOOLS}/gensyms.py ptrtab-in ${lt}/ptrtab.c ${BC_INPUTS}/nmptrs.txt @${lt}/recs.txt
        DEPENDS ${recs} ${TOOLS}/gensyms.py ${BC_INPUTS}/nmptrs.txt
        COMMENT "${name}: pointer table (gensyms.py ptrtab-in)" VERBATIM)
    add_library(${name}_tables OBJECT ${lt}/stubs.c ${lt}/copytab.c ${lt}/ptrtab.c)
    target_include_directories(${name}_tables PRIVATE ${HOST_INC})
    target_compile_options(${name}_tables PRIVATE /w /GS- ${BC_OPT})
    set(${name}_LINK ${pinned} ${lt}/abs_syms.obj $<TARGET_OBJECTS:${name}_tables> PARENT_SCOPE)
    set(${name}_REPORT ${lt}/link_report.txt PARENT_SCOPE)
endfunction()

# the exes: a fixed base below N64 memory (RIP-relative reach to 0x80000000), no
# relocations, no ASLR (every host address the game stores is below 4 GB), no
# identical-code folding (game functions keep their own addresses), a PDB and a
# map (-> EXE.syms, `nm -n` style: --syms, compare.py)
set(BC_LINK_OPTS /BASE:${BC_IMAGE_BASE} /FIXED /DYNAMICBASE:NO /HIGHENTROPYVA:NO /LARGEADDRESSAWARE
    /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:NOICF /FUNCTIONPADMIN:8 /IGNORE:4281)
function(bc_exe name)
    cmake_parse_arguments(A "" "REPORT" "SOURCES;OPTS;LIBS" ${ARGN})
    add_executable(${name} ${A_SOURCES})
    target_link_options(${name} PRIVATE ${BC_LINK_OPTS} /MAP:$<TARGET_FILE_DIR:${name}>/${name}.map ${A_OPTS})
    target_link_libraries(${name} PRIVATE ${A_LIBS})
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${PY} ${TOOLS}/mapsyms.py $<TARGET_FILE_DIR:${name}>/${name}.map
            $<TARGET_FILE_DIR:${name}>/${name}.syms
        COMMAND ${CMAKE_COMMAND} -E cat ${A_REPORT}
        VERBATIM)
endfunction()

# ---- bc_headless.exe ---------------------------------------------------------------
set(HL_LIBS bc_hd bc_fe bc_ul bc_ulfe bc_plat)
link_tables(bc_headless LIBS ${HL_LIBS} HOST bc_core bc_host bc_headless_main)
bc_exe(bc_headless REPORT ${bc_headless_REPORT} OPTS /SUBSYSTEM:CONSOLE LIBS winmm
    SOURCES $<TARGET_OBJECTS:bc_core> $<TARGET_OBJECTS:bc_host> $<TARGET_OBJECTS:bc_headless_main>
        ${bc_headless_LINK})

# ---- bc_loadcheck.exe (loadcheck_main.c: the load layer through the game's loaders) ----
link_tables(bc_loadcheck LIBS bc_lc HOST bc_core bc_lc_main)
bc_exe(bc_loadcheck REPORT ${bc_loadcheck_REPORT} OPTS /SUBSYSTEM:CONSOLE
    SOURCES $<TARGET_OBJECTS:bc_core> $<TARGET_OBJECTS:bc_lc_main> ${bc_loadcheck_LINK})
