# Windows port, stage-0 spike: real game C (NON_MATCHING) linked into a
# 32-bit Windows exe that maps RDRAM at 0x80000000.  See port/README.md.
#
#   make -C port                 build port/build/spike.exe
#   make -C port check           run it and compare with the original code (unicorn)
#
# Needs: i686-w64-mingw32-gcc, the NON_MATCHING ELFs (make VERSION=us.v11
# NON_MATCHING=1) for addresses, the matching ELFs (build/) for the reference,
# and the ROM (ROM=..., default ../../baserom.us.v11.z64) at run time.

MAKEFLAGS += -r
.SUFFIXES:
ROOT    := ..
B       := build
VERSION := us.v11
CC      := i686-w64-mingw32-gcc
PYTHON  ?= $(shell [ -x $(ROOT)/../.env/bin/python3 ] && echo $(abspath $(ROOT)/../.env/bin/python3) || echo python3)
ROM     ?= $(firstword $(wildcard $(ROOT)/../baserom.$(VERSION).z64 $(HOME)/blastcorps/baserom.$(VERSION).z64))
# the exe is a Windows program: hand it a Windows path (WSL interop)
ROM_ARG  = $(shell wslpath -w $(abspath $(ROM)) 2>/dev/null || echo $(ROM))

# game files linked into the spike (hd_code); everything else they call is stubbed
GAME_FILES ?= 69000 69930 60D50 86ED0 75490
GAME_SRC   := $(ROOT)/src.$(VERSION)/hd_code

GAME_CFLAGS := -std=gnu89 -nostdinc -I$(ROOT) -I$(ROOT)/include -I$(ROOT)/include/2.0I \
	-I$(ROOT)/include/2.0I/PR -include include/port_ultratypes.h \
	-D_LANGUAGE_C -D_FINALROM -DNON_MATCHING -DPORT_HOST -D_MIPS_SZLONG=32 -D_MIPS_SIM=1 \
	-O2 -fno-strict-aliasing -fwrapv -msse2 -mfpmath=sse -fno-common \
	-fno-builtin-sinf -fno-builtin-cosf -fno-builtin-sin -fno-builtin-cos \
	-Wno-unknown-pragmas -Wno-builtin-declaration-mismatch
PORT_CFLAGS := -O2 -msse2 -mfpmath=sse -Wall -Isrc
# 0x80000000 must be free: large-address-aware, and a fixed image base with no
# relocations (absolute symbols must never be rebased)
LDFLAGS := -Wl,--large-address-aware -Wl,--disable-dynamicbase -Wl,--disable-reloc-section

NM_ELFS   := $(ROOT)/build_nm/hd_code.$(VERSION).elf $(ROOT)/build_nm/hd_front_end.$(VERSION).elf
GAME_OBJS := $(GAME_FILES:%=$(B)/game/%.o)
GAME_RECS := $(GAME_FILES:%=$(B)/game/%.rec)
MAIN      ?= spike_main
# byte order on load (port/src/load/, see port_load.h)
LOAD_SRCS := src/load/swap.c src/load/port_assets.c src/load/gfx_fix.c
PORT_SRCS := src/rdram.c src/inflate.c src/$(MAIN).c $(LOAD_SRCS)
PORT_OBJS := $(PORT_SRCS:src/%.c=$(B)/port/%.o) $(B)/swaptab.o
PORT_HDRS := src/rdram.h src/load/port_load.h src/load/schemas.h src/platform/port_dma.h

all: $(B)/spike.exe

$(B)/addrs.txt: $(NM_ELFS) tools/gensyms.py
	@mkdir -p $(@D)
	cd $(ROOT) && $(PYTHON) port/tools/gensyms.py addrs port/$@

$(B)/game/%.s: $(GAME_SRC)/%.c include/port_ultratypes.h Makefile
	@mkdir -p $(@D)
	$(CC) $(GAME_CFLAGS) -S -o $@ $<

# pin the file's data objects to their N64 addresses (tools/relabel.py)
$(B)/game/%.pinned.s $(B)/game/%.rec: $(B)/game/%.s $(B)/addrs.txt tools/relabel.py
	$(PYTHON) tools/relabel.py $(B)/addrs.txt $< $(B)/game/$*.pinned.s $(B)/game/$*.rec

$(B)/game/%.o: $(B)/game/%.pinned.s
	$(CC) -c -o $@ $<

$(B)/port/%.o: src/%.c $(PORT_HDRS)
	@mkdir -p $(@D)
	$(CC) $(PORT_CFLAGS) -c -o $@ $<

# byte-order schema of the data images: C declarations (DWARF) + the widths
# the code reads them at (data/image_widths.txt, from tools/m64widths.py)
$(B)/typemap.txt: $(B)/addrs.txt tools/typemap.py
	cd $(ROOT) && $(PYTHON) port/tools/typemap.py port/$(B)/addrs.txt port/$@ > port/$(B)/typemap_report.txt
SWAPTAB_DATA := data/image_widths.txt data/image_static.txt data/image_overrides.txt
$(B)/swaptab.c: $(B)/typemap.txt $(SWAPTAB_DATA) tools/swaptab.py $(NM_ELFS)
	cd $(ROOT) && $(PYTHON) port/tools/swaptab.py port/$(B)/typemap.txt port/data/image_widths.txt port/$@ \
		port/$(B)/swaptab_report.txt --static port/data/image_static.txt port/$(B)/addrs.txt \
		> port/$(B)/swaptab_coverage.txt
	@cat $(B)/swaptab_coverage.txt
$(B)/swaptab.o: $(B)/swaptab.c src/load/port_load.h
	$(CC) $(PORT_CFLAGS) -c -o $@ $<

$(B)/abs_syms.ld $(B)/stubs.c &: $(GAME_OBJS) $(PORT_OBJS) tools/gensyms.py
	cd $(ROOT) && $(PYTHON) port/tools/gensyms.py link port/$(B) $(addprefix port/,$(GAME_OBJS) $(PORT_OBJS))

$(B)/copytab.c: $(GAME_RECS) $(B)/addrs.txt tools/gensyms.py
	$(PYTHON) tools/gensyms.py copytab $@ $(B)/addrs.txt $(GAME_RECS)

$(B)/stubs.o: $(B)/stubs.c
	$(CC) -O2 -w -c -o $@ $<
$(B)/copytab.o: $(B)/copytab.c $(PORT_HDRS)
	$(CC) $(PORT_CFLAGS) -c -o $@ $<

$(B)/spike.exe: $(GAME_OBJS) $(PORT_OBJS) $(B)/stubs.o $(B)/copytab.o $(B)/abs_syms.ld
	$(CC) -o $@ $(GAME_OBJS) $(PORT_OBJS) $(B)/stubs.o $(B)/copytab.o $(B)/abs_syms.ld $(LDFLAGS)

check: $(B)/spike.exe
	$(B)/spike.exe '$(ROM_ARG)' typed > $(B)/out_typed.txt
	$(B)/spike.exe '$(ROM_ARG)' raw > $(B)/out_raw.txt
	cd $(ROOT) && $(PYTHON) port/tools/n64ref.py build > port/$(B)/out_ref.txt
	$(PYTHON) tools/cmpout.py $(B)/out_ref.txt $(B)/out_typed.txt $(B)/out_raw.txt

# every hd_code game file (not the libultra ul_* wrappers) in one exe
ALL_GAME := $(sort $(basename $(notdir $(filter-out $(GAME_SRC)/ul_%,$(wildcard $(GAME_SRC)/*.c)))))
linkall:
	$(MAKE) B=build/all MAIN=linkall_main GAME_FILES="$(ALL_GAME)" build/all/spike.exe
	build/all/spike.exe '$(ROM_ARG)'

# byte-order schema of the initialised image: coverage report
typemap: $(B)/swaptab.c
	@cat $(B)/typemap_report.txt

# the byte-order layer against the original code: real assets through the
# game's loaders and consumers, native vs unicorn (src/load/loadcheck_main.c,
# tools/loadref.py)
#   LC_TRACE=trace.txt (tools/m64widths.py output) adds T9: every traced read
LC_FILES := 00000 46C20 17A70 53220 7F8B0 5BF40 60F60 5CB60 60D50
loadcheck:
	$(MAKE) B=build/lc MAIN=loadcheck_main GAME_FILES="$(LC_FILES)" build/lc/spike.exe
	@if [ -n "$(LC_TRACE)" ]; then $(PYTHON) tools/widths.py facts build/lc/facts.txt $(LC_TRACE); fi
	build/lc/spike.exe '$(ROM_ARG)' $$([ -f build/lc/facts.txt ] && echo 56789 build/lc/facts.txt) \
		> build/lc/out_native.txt
	cd $(ROOT) && $(PYTHON) port/tools/loadref.py $(abspath $(ROM)) > port/build/lc/out_ref.txt
	@grep '^T9 total' build/lc/out_native.txt || true
	$(PYTHON) tools/cmpout.py build/lc/out_ref.txt build/lc/out_native.txt

ucode:
	cd $(ROOT) && $(PYTHON) port/tools/ucode_id.py build

# ---------------------------------------------------------------------------
# headless (stage 3): every game file of hd_code and the front end, the
# ultralib gu/libm/libc/libaudio objects, and the platform layer
# (src/platform/) in build/headless/bc_headless.exe.  See README.md ("Headless").
#
#   make -C port headless
#   build/headless/bc_headless.exe ROM --frames N [--dump F,..] [--trace FILE] ...
H := build/headless
FE_SRC := $(ROOT)/src.$(VERSION)/hd_front_end
UL_DIR := $(ROOT)/../lib/ultralib
# hd_code files that are only SDK hand-written asm (GLOBAL_ASM: empty natively)
# or SDK C poking RCP registers; the platform layer replaces them
HL_HD_SDK := 8FD90 906E0 91820 91F50 91F80 92EA0 95060 96300 96F70 96FF0 971D0 971F0 972A0 \
	97B70 98330 9DB70 9DB80 9E6F0 A0A30
HL_HD := $(filter-out $(HL_HD_SDK),$(ALL_GAME))
HL_FE := $(sort $(basename $(notdir $(filter-out $(FE_SRC)/ul_%,$(wildcard $(FE_SRC)/*.c)))))
# ultralib objects compiled natively (gu, libm, libc's printf, libaudio).  The
# os/io/si/vi/pi/sp objects are replaced by the platform layer.
HL_UL := ul_lookat ul_lookatref ul_mtxcatf ul_mtxcatl ul_mtxutil ul_normalize ul_ortho \
	ul_perspective ul_rotate ul_scale ul_translate ul_sins ul_coss \
	ul_sprintf ul_xlitob ul_ldiv \
	ul_auxbus ul_bnkf ul_cents2ratio ul_copy ul_cseq ul_cspgetstate ul_cspgettempo ul_csplayer \
	ul_cspplay ul_cspsetbank ul_cspsetseq ul_cspsettempo ul_cspsetvol ul_cspstop ul_drvrnew \
	ul_env ul_event ul_filter ul_heapalloc ul_heapinit ul_load ul_mainbus ul_resample ul_reverb \
	ul_save ul_seq ul_seqplayer ul_sl ul_synaddplayer ul_synallocfx ul_synallocvoice \
	ul_synfreevoice ul_synsetfxmix ul_synsetpan ul_synsetpitch ul_synsetpriority ul_synsetvol \
	ul_synstartvoice ul_synstartvoiceparam ul_synstopvoice ul_synthesizer
# ... plus four with a big-endian view of a double, fixed up by sed (below)
HL_UL_LE := ul_xprintf ul_xldtob ul_sinf ul_cosf
# platform files that see the game's <ultra64.h>
HL_PLAT := os_thread os_time os_hw os_si input plat_core save pif
# host-side platform files (<windows.h>, the C runtime)
HL_HOST := plat_host headless_main
# the load layer (byte order, port_on_dma) lives in src/load/; without it,
# port_on_dma is the identity stub
HL_LOAD := $(basename $(notdir $(wildcard src/load/*.c)))
ifeq ($(HL_LOAD),)
HL_HOST += port_dma_stub
endif

# Struct layouts must match the N64's (IDO, o32):
#  -malign-double     u64/f64 struct members 8-aligned (OSThread's context,
#                     OSTimer, the scheduler's OSTime fields ...)
#  -mno-ms-bitfields  GCC bitfield packing; MinGW's default MS layout makes
#                     the GBI's Gfx union 16 bytes instead of 8
HL_ABI := -malign-double -mno-ms-bitfields
# -fno-optimize-sibling-calls: a call's return address names its real caller
# (the keys of --clock and --sync, see os_time.c / os_thread.c; crash reports)
# HL_EXTRA: extra game-file flags for debugging (e.g. -fno-inline for --calls
# arguments; delete build/headless/game/*/*.s first, make doesn't see flags)
HL_CFLAGS := $(GAME_CFLAGS) -DPORT_HOST $(HL_ABI) -g -fno-optimize-sibling-calls $(HL_EXTRA)
HL_UL_CFLAGS := -std=gnu89 -nostdinc -I$(UL_DIR) -I$(UL_DIR)/include -I$(UL_DIR)/include/compiler/modern_gcc \
	-I$(UL_DIR)/include/PR -I$(UL_DIR)/src/libc -D_MIPS_SZLONG=32 -DBUILD_VERSION=VERSION_I \
	-DBUILD_VERSION_STRING=\"2.0I\" -DNDEBUG -D_FINALROM -DF3DEX_GBI -D_LANGUAGE_C \
	-O2 -fno-strict-aliasing -fwrapv -msse2 -mfpmath=sse $(HL_ABI) -mlong-double-64 -fno-common \
	-fno-builtin -g -w
HL_PLAT_CFLAGS := $(HL_CFLAGS) -Isrc/platform -Wall -Wno-unknown-pragmas
HL_HOST_CFLAGS := -O2 -msse2 -mfpmath=sse -Wall -Isrc -Isrc/platform -g

# the front end's own SDK objects used natively: 0E7B0 calls __osContDataCrc,
# and its Controller Pak file system runs over the PIF emulation (pif.c)
HL_ULFE := ul_crc ul_contpfs ul_contramread ul_contramwrite ul_pfsallocatefile ul_pfschecker \
	ul_pfsdeletefile ul_pfsfilestate ul_pfsfreeblocks ul_pfsinitpak ul_pfsisplug ul_pfsreadwritefile \
	ul_pfssearchfile

HL_GAME_OBJS := $(HL_HD:%=$(H)/game/hd/%.o) $(HL_FE:%=$(H)/game/fe/%.o) \
	$(HL_UL:%=$(H)/game/ul/%.o) $(HL_UL_LE:%=$(H)/game/ul/%.o) $(HL_ULFE:%=$(H)/game/ulfe/%.o) \
	$(HL_PLAT:%=$(H)/game/plat/%.o)
HL_RECS := $(HL_GAME_OBJS:.o=.rec)
# audio: the microcode interpreter and the AI output (src/audio/)
HL_AUDIO := aspmain port_audio
HL_HOST_OBJS := $(HL_HOST:%=$(H)/host/%.o) $(H)/host/rdram.o $(H)/host/inflate.o \
	$(HL_LOAD:%=$(H)/host/load/%.o) $(if $(HL_LOAD),$(H)/host/swaptab.o,) $(HL_AUDIO:%=$(H)/host/audio/%.o)

$(H)/addrs.txt: $(NM_ELFS) tools/gensyms.py
	@mkdir -p $(@D)
	cd $(ROOT) && $(PYTHON) port/tools/gensyms.py addrs port/$@

$(H)/game/hd/%.s: $(GAME_SRC)/%.c include/port_ultratypes.h Makefile
	@mkdir -p $(@D)
	$(CC) $(HL_CFLAGS) $(HL_GAME_INSTR) -S -o $@ $<
# game functions call __cyg_profile_func_enter (os_thread.c): with --sync,
# the entries of the functions that read the game's retrace counter are
# switch points too (port/tools/cmp_spec.py lists them)
HL_GAME_INSTR := -finstrument-functions
# the callers of osGetTime/osGetCount keep their own functions (no inlining),
# so --clock can give each caller the emulator's values (os_time.c)
HL_CLOCK_CALLERS := 00000 1C460 20460 22EE0 23C20 2C560
$(HL_CLOCK_CALLERS:%=$(H)/game/hd/%.s): HL_CFLAGS += -fno-inline
$(H)/game/fe/%.s: $(FE_SRC)/%.c include/port_ultratypes.h Makefile
	@mkdir -p $(@D)
	$(CC) $(HL_CFLAGS) $(HL_GAME_INSTR) -S -o $@ $<
$(H)/game/ul/%.s: $(GAME_SRC)/%.c Makefile
	@mkdir -p $(@D)
	$(CC) $(HL_UL_CFLAGS) -S -o $@ $<
$(H)/game/ulfe/%.s: $(FE_SRC)/%.c $(H)/le/PRinternal/controller.h Makefile
	@mkdir -p $(@D)
	$(CC) -I$(H)/le $(HL_UL_CFLAGS) -S -o $@ $<
# little-endian host: pif.c delivers the pak's system blocks with host-order
# u16s, so the byte pairs the SDK also reads as one u16 swap places:
# __OSInodeUnit's (bank, page) view of ipage, and __OSPackId's (banks,
# version), which __osIdCheckSum sums as a u16 word
$(H)/le/PRinternal/controller.h: $(UL_DIR)/include/PRinternal/controller.h Makefile
	@mkdir -p $(@D)
	sed -e '/\/\* 0x0 \*\/ u8 bank;/{N;s/\(.*\)u8 bank;\(.*\)\n\(.*\)u8 page;/\1u8 page;\2\n\3u8 bank;/}' \
	    -e '/u8 banks;/{N;s/\(.*\)u8 banks;\(.*\)\n\(.*\)u8 version;/\1u8 version;\2\n\3u8 banks;/}' $< > $@
	grep -A1 'u8 page;' $@ | grep -q 'u8 bank;'
	grep -A1 'u8 version;' $@ | grep -q 'u8 banks;'
# osPfsAllocateFile (this SDK) never sets the new directory entry's status and
# reserved bytes: they are stack leftovers. On the N64 (mupen64plus, original
# ROM) status is 2 (later SDKs' DIR_STATUS_OCCUPIED), reserved 0 for the
# first file Blast Corps creates and 3 for the next ones; the native build
# stores 2 and 0 (nothing reads them)
$(H)/le/pfsallocatefile.c: $(UL_DIR)/src/io/pfsallocatefile.c Makefile
	@mkdir -p $(@D)
	sed -e 's/^\( *\)dir\.data_sum = 0;/\1dir.data_sum = 0;\n\1dir.status = 2; dir.reserved = 0; \/* port: the N64 stack leftovers *\//' $< > $@
	grep -q 'dir.reserved = 0' $@
$(H)/game/ulfe/ul_pfsallocatefile.s: $(H)/le/pfsallocatefile.c $(H)/le/PRinternal/controller.h
	@mkdir -p $(@D)
	$(CC) -I$(H)/le -I$(UL_DIR) $(HL_UL_CFLAGS) -S -o $@ $<
$(H)/game/plat/%.s: src/platform/%.c src/platform/plat.h src/platform/plat_host.h src/platform/port_dma.h \
		src/audio/port_audio.h Makefile
	@mkdir -p $(@D)
	$(CC) $(HL_PLAT_CFLAGS) -S -o $@ $<

# little-endian host: the sign/exponent half-word of a double is the last one
$(H)/le/xldtob.c: $(UL_DIR)/src/libc/xldtob.c Makefile
	@mkdir -p $(@D)
	sed -e 's/^#define _D0 0/#define _D0 3/' -e 's/^#define _D1 1/#define _D1 2/' \
	    -e 's/^#define _D2 2/#define _D2 1/' -e 's/^#define _D3 3/#define _D3 0/' $< > $@
	grep -q '^#define _D0 3' $@ && grep -q '^#define _D3 0' $@
$(H)/le/xprintf.c: $(GAME_SRC)/ul_xprintf.c Makefile
	@mkdir -p $(@D)
	sed -e 's/(((unsigned short \*)&(x))\[0\] & 0x8000)/(((unsigned short *)\&(x))[3] \& 0x8000)/' $< > $@
	grep -q 'short \*)&(x))\[3\]' $@
# sinf/cosf: their constants are `du` unions ({hi word, lo word} of a double,
# guint.h) initialised in big-endian order: swap each pair (found by
# port/tools/compare.py: guRotate matrices came out NaN)
$(H)/le/%f.c: $(UL_DIR)/src/gu/%f.c Makefile
	@mkdir -p $(@D)
	sed -E 's/\{(0x[0-9a-fA-F]{8}),[[:space:]]*(0x[0-9a-fA-F]{8})\}/{\2, \1}/' $< > $@
	grep -q '{0x00000000, 0x3ff00000}' $@
$(H)/game/ul/ul_sinf.s: $(H)/le/sinf.c
	@mkdir -p $(@D)
	$(CC) $(HL_UL_CFLAGS) -I$(UL_DIR)/src/gu -S -o $@ $<
$(H)/game/ul/ul_cosf.s: $(H)/le/cosf.c
	@mkdir -p $(@D)
	$(CC) $(HL_UL_CFLAGS) -I$(UL_DIR)/src/gu -S -o $@ $<
$(H)/game/ul/ul_xldtob.s: $(H)/le/xldtob.c
	@mkdir -p $(@D)
	$(CC) $(HL_UL_CFLAGS) -S -o $@ $<
$(H)/game/ul/ul_xprintf.s: $(H)/le/xprintf.c
	@mkdir -p $(@D)
	$(CC) $(HL_UL_CFLAGS) -S -o $@ $<

$(H)/%.pinned.s $(H)/%.rec: $(H)/%.s $(H)/addrs.txt tools/relabel.py
	$(PYTHON) tools/relabel.py $(H)/addrs.txt $< $(H)/$*.pinned.s $(H)/$*.rec

$(H)/game/%.o: $(H)/game/%.pinned.s
	$(CC) -c -o $@ $<

$(H)/host/%.o: src/platform/%.c src/platform/plat_host.h src/platform/port_dma.h src/rdram.h \
		src/audio/port_audio.h Makefile
	@mkdir -p $(@D)
	$(CC) $(HL_HOST_CFLAGS) -c -o $@ $<
$(H)/host/audio/%.o: src/audio/%.c src/audio/aspmain.h src/audio/port_audio.h src/platform/plat_host.h src/rdram.h \
		Makefile
	@mkdir -p $(@D)
	$(CC) $(HL_HOST_CFLAGS) -Wextra -Wno-unused-parameter -c -o $@ $<
$(H)/host/%.o: src/%.c src/rdram.h Makefile
	@mkdir -p $(@D)
	$(CC) $(HL_HOST_CFLAGS) -c -o $@ $<
$(H)/typemap.txt: $(H)/addrs.txt tools/typemap.py
	cd $(ROOT) && $(PYTHON) port/tools/typemap.py port/$(H)/addrs.txt port/$@ > /dev/null
$(H)/host/load/%.o: src/load/%.c src/rdram.h src/platform/port_dma.h $(wildcard src/load/*.h) Makefile
	@mkdir -p $(@D)
	$(CC) $(HL_HOST_CFLAGS) -Isrc/load -c -o $@ $<
# the load layer's generated image swap table (see $(B)/swaptab.c above)
$(H)/swaptab.c: $(H)/typemap.txt $(SWAPTAB_DATA) tools/swaptab.py $(NM_ELFS)
	cd $(ROOT) && $(PYTHON) port/tools/swaptab.py port/$(H)/typemap.txt port/data/image_widths.txt port/$@ \
		port/$(H)/swaptab_report.txt --static port/data/image_static.txt port/$(H)/addrs.txt \
		> port/$(H)/swaptab_coverage.txt
	@cat $(H)/swaptab_coverage.txt
$(H)/host/swaptab.o: $(H)/swaptab.c src/load/port_load.h
	@mkdir -p $(@D)
	$(CC) $(HL_HOST_CFLAGS) -c -o $@ $<

$(H)/abs_syms.ld $(H)/stubs.c &: $(HL_GAME_OBJS) $(HL_HOST_OBJS) tools/gensyms.py
	cd $(ROOT) && $(PYTHON) port/tools/gensyms.py link port/$(H) $(addprefix port/,$(HL_GAME_OBJS) $(HL_HOST_OBJS))

$(H)/copytab.c: $(HL_RECS) $(H)/addrs.txt tools/gensyms.py
	$(PYTHON) tools/gensyms.py copytab $@ $(H)/addrs.txt $(HL_RECS)

$(H)/stubs.o: $(H)/stubs.c
	$(CC) -O2 -w -c -o $@ $<
$(H)/copytab.o: $(H)/copytab.c src/rdram.h
	$(CC) $(PORT_CFLAGS) -c -o $@ $<

$(H)/bc_headless.exe: $(HL_GAME_OBJS) $(HL_HOST_OBJS) $(H)/stubs.o $(H)/copytab.o $(H)/abs_syms.ld
	$(CC) -o $@ $(HL_GAME_OBJS) $(HL_HOST_OBJS) $(H)/stubs.o $(H)/copytab.o $(H)/abs_syms.ld $(LDFLAGS) \
		-Wl,-Map=$(H)/bc_headless.map
	$(CC:gcc=nm) -n $@ > $(H)/bc_headless.syms
	@cat $(H)/link_report.txt | head -3

headless: $(H)/bc_headless.exe

# ---------------------------------------------------------------------------
# windowed (stage 4): bc_headless's game and platform objects plus src/live/
# (SDL2 window, input, RT64 renderer) in build/game/bc.exe, with the run-time
# DLLs next to it.  RT64 is built by rt64/build_rt64.sh into $(TP)/rt64_bc
# (pinned commit, patches in rt64/patches).  See README.md ("Windowed").
#
#   make -C port game
#   build/game/bc.exe ROM [bc_headless options] [--api d3d12|vulkan] [--shot F,..] ...
G := build/game
TP ?= $(HOME)/thirdparty
RT64_W := $(TP)/rt64_bc
RT64_SRC := $(RT64_W)/src
RT64_B := $(RT64_W)/build-i686
CXX := i686-w64-mingw32-g++-posix
RT64_LIBS := $(RT64_B)/rt64.a $(RT64_B)/src/contrib/plume/libplume.a $(RT64_B)/src/contrib/re-spirv/libre-spirv.a \
	$(RT64_B)/src/contrib/nativefiledialog-extended/src/libnfd.a $(RT64_B)/src/contrib/zstd/build/cmake/lib/libzstd.a
RT64_DLLS := dxcompiler.dll dxil.dll SDL2.dll
SDL_DIR := $(RT64_SRC)/src/contrib/mupen64plus-win32-deps/SDL2-2.26.3
LIVE_CXXFLAGS := -O2 -std=gnu++17 -g -Wall -Wno-unknown-pragmas -Wno-attributes \
	-DRT64_NATIVE_HOST_LAYOUT -DSDL_MAIN_HANDLED -DFFX_GCC -DHLSL_CPU -DIMGUI_IMPL_VULKAN_NO_PROTOTYPES -DNOMINMAX \
	-D__PRFCHWINTRIN_H -DImTextureID=ImU64 -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00 -D__REQUIRED_RPCNDR_H_VERSION__=475 \
	-include $(RT64_W)/shim/rt64_mingw_compat.h -I$(RT64_W)/shim \
	-I$(TP)/DirectX-Headers/include/directx -I$(TP)/DirectX-Headers/include/dxguids -I$(TP)/DirectX-Headers/include \
	-I$(SDL_DIR)/include -I$(RT64_SRC)/src -I$(RT64_SRC)/src/contrib -I$(RT64_SRC)/src/contrib/plume \
	-I$(RT64_SRC)/src/contrib/imgui -I$(RT64_SRC)/src/contrib/hlslpp/include -I$(RT64_SRC)/src/contrib/dxc/inc \
	-I$(RT64_B)/src -I$(RT64_SRC)/src/contrib/plume/contrib/volk \
	-I$(RT64_SRC)/src/contrib/plume/contrib/Vulkan-Headers/include \
	-I$(RT64_SRC)/src/contrib/plume/contrib/VulkanMemoryAllocator/include \
	-I$(RT64_SRC)/src/contrib/plume/contrib/D3D12MemoryAllocator/include \
	-Isrc -Isrc/platform
LIVE_SYSLIBS := -ld3d12 -ld3dcompiler -ldxgi -lshcore -ldxguid -lole32 -loleaut32 -luuid -lshell32 -lshlwapi \
	-lcomdlg32 -lgdi32 -luser32 -ldwmapi -limm32 -lversion -lsetupapi -lwinmm
# the same objects as bc_headless, but the host entry comes from src/live/
# (live_setup.cpp: bc.ini, bindings, the ROM dialog and check), plus the exe's
# icon and version resource (res/bc.rc)
G_HOST_OBJS := $(filter-out $(H)/host/headless_main.o,$(HL_HOST_OBJS)) $(G)/host/headless_main.o \
	$(G)/live/live_rt64.o $(G)/live/live_setup.o $(G)/bc_res.o
WINDRES := i686-w64-mingw32-windres
# the version the exe reports (log, file properties, dist folder name)
GIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null || echo dev)
GIT_DIRTY := $(shell git diff --quiet HEAD -- . 2>/dev/null || echo -modified)
BC_VERSION := $(GIT_HASH)$(GIT_DIRTY)

$(G)/rt64.stamp: rt64/build_rt64.sh $(wildcard rt64/patches/*.patch)
	@mkdir -p $(@D)
	TP=$(TP) JOBS=$${JOBS:-2} bash rt64/build_rt64.sh
	touch $@
# rewritten only when the version changes (the objects that show it rebuild)
$(G)/version.h: FORCE
	@mkdir -p $(@D)
	@printf '#define BC_VERSION "%s"\n' '$(BC_VERSION)' > $@.tmp
	@cmp -s $@.tmp $@ && rm $@.tmp || mv $@.tmp $@
$(G)/host/headless_main.o: src/platform/headless_main.c src/platform/plat_host.h src/rdram.h Makefile
	@mkdir -p $(@D)
	$(CC) $(HL_HOST_CFLAGS) -DPORT_LIVE -c -o $@ $<
$(G)/live/%.o: src/live/%.cpp src/live/live_setup.h src/platform/plat_host.h src/rdram.h $(G)/version.h \
		$(G)/rt64.stamp Makefile
	@mkdir -p $(@D)
	$(CXX) $(LIVE_CXXFLAGS) -include $(G)/version.h -c -o $@ $<
$(G)/bc.ico: tools/make_icon.py
	@mkdir -p $(@D)
	$(PYTHON) tools/make_icon.py $@
$(G)/bc_res.o: res/bc.rc $(G)/bc.ico $(G)/version.h
	$(WINDRES) -I$(G) -Ires -O coff -o $@ $<
# -mwindows: a GUI program (no console window when double-clicked); the log
# goes to stderr when there is one, else to bc.log (live_rt64.cpp)
$(G)/bc.exe: $(HL_GAME_OBJS) $(G_HOST_OBJS) $(H)/stubs.o $(H)/copytab.o $(H)/abs_syms.ld $(G)/rt64.stamp
	$(CXX) -o $@ $(HL_GAME_OBJS) $(G_HOST_OBJS) $(H)/stubs.o $(H)/copytab.o $(H)/abs_syms.ld $(RT64_LIBS) \
		$(RT64_W)/dll/SDL2.dll $(RT64_W)/dll/libdxcompiler.a $(LIVE_SYSLIBS) \
		-static -static-libgcc -static-libstdc++ -mwindows $(LDFLAGS) -Wl,-Map=$(G)/bc.map
	$(CC:gcc=nm) -n $@ > $(G)/bc.syms
	cp -u $(addprefix $(RT64_W)/dll/,$(RT64_DLLS)) $(G)/

game: $(G)/bc.exe

# ---------------------------------------------------------------------------
# the player package: build/dist/BlastCorps-port-<hash>/ (bc.exe, its DLLs,
# README.txt, THIRD_PARTY_LICENSES.txt) and a zip of it.  No ROM data: the
# game reads the player's own ROM at run time.
DIST_NAME := BlastCorps-port-$(BC_VERSION)
DIST := build/dist/$(DIST_NAME)
dist: $(G)/bc.exe dist/README.txt dist/third_party.sh
	rm -rf $(DIST) $(DIST).zip
	mkdir -p $(DIST)
	cp $(G)/bc.exe $(addprefix $(G)/,$(RT64_DLLS)) $(DIST)/
	$(CC:gcc=strip) --strip-debug $(DIST)/bc.exe
	sed -e 's/@VERSION@/$(BC_VERSION)/' dist/README.txt | sed -e 's/$$/\r/' > $(DIST)/README.txt
	TP=$(TP) RT64_SRC=$(RT64_SRC) bash dist/third_party.sh | sed -e 's/$$/\r/' > $(DIST)/THIRD_PARTY_LICENSES.txt
	cd build/dist && $(PYTHON) -m zipfile -c $(DIST_NAME).zip $(DIST_NAME)/
	@ls -l $(DIST) $(DIST).zip

FORCE:

# frame-by-frame comparison with the emulator over attract demo DEMO
# (tools/compare.py; the emulator run is cached in CMP_CACHE)
DEMO ?= 0
CMP_CACHE ?= $(HOME)/cmp_cache
CMP_KIND ?= nm
compare: $(H)/bc_headless.exe
	$(PYTHON) tools/compare.py run --demo $(DEMO) --cache $(CMP_CACHE) --kind $(CMP_KIND) $(CMP_ARGS)

# quick regression check: loadcheck, then bc_headless following a cached
# emulator run (NM test ROM, rebuilt first; LLE audio and visibility tests) of
# the first VERIFY_FRAMES frames: timeline, RAM dumps (lenient and strict),
# visibility tests and audio tasks against the emulator's; fails when a number
# is worse than in data/verify_expect.txt (tools/compare.py verify)
VERIFY_FRAMES ?= 1500
verify:
	$(MAKE) -C $(ROOT) VERSION=$(VERSION) NON_MATCHING=1 nmrom BASEROM=$(abspath $(ROM)) > /dev/null
	$(MAKE) $(H)/bc_headless.exe $(H)/typemap_all.txt
	$(MAKE) loadcheck
	$(PYTHON) tools/compare.py verify --frames $(VERIFY_FRAMES) --cache $(CMP_CACHE) $(VERIFY_ARGS)

# the strict (typed) comparison's layouts: every symbol's declared leaves,
# and with LC_TRACE (tools/m64widths.py output) the traced reads of every asset
strict-data: $(H)/typemap_all.txt
	@if [ -n "$(LC_TRACE)" ]; then WIDTHS_NO_DECODERS=1 $(PYTHON) tools/widths.py facts $(H)/facts.txt $(LC_TRACE); fi
$(H)/typemap_all.txt: $(H)/addrs.txt tools/typemap.py
	cd $(ROOT) && TYPEMAP_ALL=1 $(PYTHON) port/tools/typemap.py port/$(H)/addrs.txt port/$@ > /dev/null

clean:
	rm -rf $(B)

.PHONY: all check clean linkall loadcheck typemap ucode headless compare game strict-data dist verify FORCE
.SECONDARY:
