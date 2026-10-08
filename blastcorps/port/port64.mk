# ---------------------------------------------------------------------------
# 64-bit (x86_64) build: the same game, platform and host sources as
# headless/game, compiled with clang for x86_64-w64-mingw32 and linked with
# mingw-w64's x86_64 binutils/CRT.  N64 memory stays at 0x80000000 with N64
# layouts (4-byte pointers: N64P, port/include/port_n64ptr.h).  See README.md
# ("64-bit").  Included by Makefile.
#
#   make -C port headless64          build/headless64/bc_headless.exe
#   make -C port game64 dist64       build/game64/bc.exe, its package
#   make -C port ptrcheck64          pointer-width audit + record layouts (i686 vs x86_64)
#   make -C port verify64 / verify-levels64 / loadcheck64
H64 := build/headless64
CC64 := clang-18 --target=x86_64-w64-mingw32
LD64 := x86_64-w64-mingw32-gcc
NM64 := x86_64-w64-mingw32-nm
# -fms-extensions: __ptr32 (N64P); -mno-ms-bitfields: GCC bitfield packing (the
# GBI's Gfx); -fdata-sections: one section per object, for coffpin.py;
# -fno-auto-import: direct (RIP-relative) access to data, no .refptr stubs;
# -ffp-contract=off: no FMA contraction (the i686 build has none either).
# Errors: a pointer that changes address space under another pointer
# (`u8 **p = D_x` with D_x an N64P array) and pointer/integer mixups.
# Not warned: int <-> pointer casts of 32-bit values (N64 addresses); the
# dangerous kind, a signed 32-bit int cast to a pointer (it sign-extends:
# N64_IPTR), is what ptrcheck64 reports as SEXT.
C64_COMMON := -fms-extensions -mno-ms-bitfields -fdata-sections -fno-addrsig -fno-auto-import -ffp-contract=off \
	-Werror=incompatible-pointer-types -Werror=int-conversion -Wno-pointer-to-int-cast \
	-Wno-void-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-int-to-void-pointer-cast
# Code that sees N64 data is compiled through LLVM IR: the x86-64 ABI gives a
# global array of 16 bytes or more 16-byte alignment and clang assumes it for
# every array, extern ones too, so it used movaps on D_8036B8C8 (an N64 array at
# an 8-aligned address: a fault).  tools/llalign64.py lowers every alignment
# above 4 on memory accesses and N64 globals in the optimised IR, then the code
# is generated (no further IR optimisation); tools/align64.py checks the
# objects: no 16-byte-aligned access except on the stack or constants.
# $(call cc64ir,FLAGS): $< -> $@
cc64ir = $(CC64) $(1) -S -emit-llvm -o $@.ll $< && \
	$(PYTHON) tools/llalign64.py $@.ll $@.a.ll $(H64)/addrs.txt && \
	$(CC64) $(1) -Wno-unused-command-line-argument -Xclang -disable-llvm-optzns -c -o $@ $@.a.ll && \
	rm -f $@.ll $@.a.ll
IR64_DEPS := tools/llalign64.py $(H64)/addrs.txt
# -fno-inline-functions: only `inline` functions are inlined.  --sync and
# --clock key the emulator's values on the calling game function (a return
# address); clang inlines far more than gcc (func_802A2BB0's PI wait came out
# inside func_802A1D54: 8627 sync mismatches in verify), so game functions
# keep their own bodies, as the N64's do.
HL64_CFLAGS := $(filter-out -msse2 -mfpmath=sse -Wno-builtin-declaration-mismatch,$(GAME_CFLAGS)) -DPORT_HOST \
	$(C64_COMMON) -g -fno-optimize-sibling-calls -fno-inline-functions -Wno-incompatible-library-redeclaration \
	$(HL_EXTRA)
# ultralib's headers come from the marked copies (ulhdr64.py) in $(H64)/ulinc
HL64_UL_CFLAGS := -I$(H64)/ulinc $(subst -I$(UL_DIR)/include,-I$(H64)/ulinc/include,$(subst \
	-I$(UL_DIR)/src/libc,-I$(H64)/ulinc/src/libc,$(filter-out -msse2 -mfpmath=sse $(HL_ABI),$(HL_UL_CFLAGS)))) \
	$(C64_COMMON) -include include/port_n64ptr.h
HL64_PLAT_CFLAGS := $(HL64_CFLAGS) -Isrc/platform -Wall -Wno-unknown-pragmas
HL64_HOST_CFLAGS := -O2 -Wall -Isrc -Isrc/platform -g -fno-addrsig -Wno-pointer-to-int-cast
# a fixed base below RDRAM (RIP-relative reach), no relocations, no ASLR: every
# host address the game can be handed (image, heap, fiber stacks) is below 4 GB
LDFLAGS64 := -Wl,--image-base,0x40000000 -Wl,--disable-dynamicbase -Wl,--disable-high-entropy-va \
	-Wl,--disable-reloc-section

HL64_GAME_OBJS := $(HL_HD:%=$(H64)/game/hd/%.o) $(HL_FE:%=$(H64)/game/fe/%.o) \
	$(HL_UL:%=$(H64)/game/ul/%.o) $(HL_UL_LE:%=$(H64)/game/ul/%.o) $(HL_ULFE:%=$(H64)/game/ulfe/%.o) \
	$(HL_PLAT:%=$(H64)/game/plat/%.o)
HL64_RECS := $(HL64_GAME_OBJS:.o=.rec)
HL64_HOST_OBJS := $(HL_HOST:%=$(H64)/host/%.o) $(H64)/host/rdram.o $(H64)/host/inflate.o \
	$(HL_LOAD:%=$(H64)/host/load/%.o) $(if $(HL_LOAD),$(H64)/host/swaptab.o,) $(HL_AUDIO:%=$(H64)/host/audio/%.o)

$(H64)/addrs.txt: $(H)/addrs.txt
	@mkdir -p $(@D)
	cp $< $@

# ultralib's public headers with the N64 pointer marks (tools/ulhdr64.py,
# data/ultralib_n64ptr.txt): the ul_* objects share these records with the game
UL64_HDRS := $(shell sed -n 's/^\([^#: ][^: ]*\):.*/\1/p' data/ultralib_n64ptr.txt 2>/dev/null | sort -u)
# ... and its sources with the function pointers kept in N64 memory used
# through N64FN_SET/N64FN_GET (tools/ulsrc64.py): $(H64)/ulsrc/ul_*.c
UL64_SRCS := $(HL_UL:%=$(GAME_SRC)/%.c) $(HL_ULFE:%=$(FE_SRC)/%.c)
$(H64)/ulinc.stamp: data/ultralib_n64ptr.txt tools/ulhdr64.py tools/ulsrc64.py $(UL64_SRCS)
	rm -rf $(H64)/ulinc $(H64)/ulsrc
	@mkdir -p $(H64)/ulinc
	$(PYTHON) tools/ulhdr64.py $(UL_DIR) data/ultralib_n64ptr.txt $(H64)/ulinc
	$(PYTHON) tools/ulsrc64.py $(UL_DIR) $(H64)/ulinc $(H64)/ulsrc $(UL64_SRCS)
	touch $@
$(H64)/ulsrc/%.c: $(H64)/ulinc.stamp
	@test -f $@

$(H64)/game/hd/%.raw.o: $(GAME_SRC)/%.c include/port_ultratypes.h include/port_n64ptr.h $(IR64_DEPS) Makefile \
		port64.mk
	@mkdir -p $(@D)
	$(call cc64ir,$(HL64_CFLAGS) $(HL_GAME_INSTR))
$(HL_CLOCK_CALLERS:%=$(H64)/game/hd/%.raw.o): HL64_CFLAGS += -fno-inline
$(H64)/game/fe/%.raw.o: $(FE_SRC)/%.c include/port_ultratypes.h include/port_n64ptr.h $(IR64_DEPS) Makefile \
		port64.mk
	@mkdir -p $(@D)
	$(call cc64ir,$(HL64_CFLAGS) $(HL_GAME_INSTR))
$(H64)/game/ul/%.raw.o: $(H64)/ulsrc/%.c $(H64)/ulinc.stamp $(IR64_DEPS) Makefile port64.mk
	@mkdir -p $(@D)
	$(call cc64ir,$(HL64_UL_CFLAGS))
$(H64)/game/ul/ul_sinf.raw.o: $(H)/le/sinf.c $(H64)/ulinc.stamp $(IR64_DEPS)
	@mkdir -p $(@D)
	$(call cc64ir,$(HL64_UL_CFLAGS) -I$(UL_DIR)/src/gu)
$(H64)/game/ul/ul_cosf.raw.o: $(H)/le/cosf.c $(H64)/ulinc.stamp $(IR64_DEPS)
	@mkdir -p $(@D)
	$(call cc64ir,$(HL64_UL_CFLAGS) -I$(UL_DIR)/src/gu)
$(H64)/game/ul/ul_xldtob.raw.o: $(H)/le/xldtob.c $(H64)/ulinc.stamp $(IR64_DEPS)
	@mkdir -p $(@D)
	$(call cc64ir,$(HL64_UL_CFLAGS))
$(H64)/game/ul/ul_xprintf.raw.o: $(H)/le/xprintf.c $(H64)/ulinc.stamp $(IR64_DEPS)
	@mkdir -p $(@D)
	$(call cc64ir,$(HL64_UL_CFLAGS))
$(H64)/game/ulfe/%.raw.o: $(H64)/ulsrc/%.c $(H)/le/PRinternal/controller.h $(H64)/ulinc.stamp $(IR64_DEPS) Makefile \
		port64.mk
	@mkdir -p $(@D)
	$(call cc64ir,-I$(H)/le $(HL64_UL_CFLAGS))
$(H64)/game/ulfe/ul_pfsallocatefile.raw.o: $(H)/le/pfsallocatefile.c $(H)/le/PRinternal/controller.h \
		$(H64)/ulinc.stamp $(IR64_DEPS)
	@mkdir -p $(@D)
	$(call cc64ir,-I$(H)/le $(HL64_UL_CFLAGS))
$(H64)/game/plat/%.raw.o: src/platform/%.c src/platform/plat.h src/platform/plat_host.h src/platform/port_dma.h \
		src/audio/port_audio.h include/port_n64ptr.h $(IR64_DEPS) Makefile port64.mk
	@mkdir -p $(@D)
	$(call cc64ir,$(HL64_PLAT_CFLAGS))

# pin the object's N64 data (tools/coffpin.py: works on the COFF object)
$(H64)/%.o $(H64)/%.rec: $(H64)/%.raw.o $(H64)/addrs.txt tools/coffpin.py
	$(PYTHON) tools/coffpin.py $(H64)/addrs.txt $< $(H64)/$*.o $(H64)/$*.rec

$(H64)/host/%.o: src/platform/%.c src/platform/plat_host.h src/platform/port_dma.h src/rdram.h \
		src/audio/port_audio.h Makefile port64.mk
	@mkdir -p $(@D)
	$(CC64) $(HL64_HOST_CFLAGS) -c -o $@ $<
$(H64)/host/audio/%.o: src/audio/%.c src/audio/aspmain.h src/audio/port_audio.h src/platform/plat_host.h src/rdram.h \
		Makefile port64.mk
	@mkdir -p $(@D)
	$(CC64) $(HL64_HOST_CFLAGS) -Wextra -Wno-unused-parameter -c -o $@ $<
$(H64)/host/%.o: src/%.c src/rdram.h Makefile port64.mk
	@mkdir -p $(@D)
	$(CC64) $(HL64_HOST_CFLAGS) -c -o $@ $<
$(H64)/host/load/%.o: src/load/%.c src/rdram.h src/platform/port_dma.h $(wildcard src/load/*.h) Makefile port64.mk
	@mkdir -p $(@D)
	$(CC64) $(HL64_HOST_CFLAGS) -Isrc/load -c -o $@ $<
$(H64)/host/swaptab.o: $(H)/swaptab.c src/load/port_load.h
	@mkdir -p $(@D)
	$(CC64) $(HL64_HOST_CFLAGS) -c -o $@ $<

# the N64 data symbols the objects use, as absolute symbols in a COFF object
# (tools/rdramobj.py), and a trap for every function nothing defines
$(H64)/abs_syms.o $(H64)/stubs.c &: $(HL64_GAME_OBJS) $(HL64_HOST_OBJS) tools/gensyms.py tools/rdramobj.py
	cd $(ROOT) && NM=$(NM64) $(PYTHON) port/tools/gensyms.py link64 port/$(H64) \
		$(addprefix port/,$(HL64_GAME_OBJS) $(HL64_HOST_OBJS))
$(H64)/copytab.c: $(HL64_RECS) $(H64)/addrs.txt tools/gensyms.py
	$(PYTHON) tools/gensyms.py copytab64 $@ $(H64)/addrs.txt $(HL64_RECS)
# every pointer slot of the pinned objects: the NON_MATCHING ELF's value
# (N64_DPTR initialisers are 0 natively)
$(H64)/ptrtab.c: $(HL64_RECS) $(NM_ELFS) tools/gensyms.py
	cd $(ROOT) && $(PYTHON) port/tools/gensyms.py ptrtab port/$@ $(addprefix port/,$(HL64_RECS))
$(H64)/stubs.o: $(H64)/stubs.c
	$(CC64) -O2 -w -c -o $@ $<
$(H64)/copytab.o: $(H64)/copytab.c src/rdram.h
	$(CC64) $(HL64_HOST_CFLAGS) -Isrc -c -o $@ $<
$(H64)/ptrtab.o: $(H64)/ptrtab.c src/rdram.h
	$(CC64) $(HL64_HOST_CFLAGS) -Isrc -c -o $@ $<

HL64_LINK_OBJS := $(HL64_GAME_OBJS) $(HL64_HOST_OBJS) $(H64)/stubs.o $(H64)/copytab.o $(H64)/ptrtab.o $(H64)/abs_syms.o
$(H64)/align.ok: $(HL64_GAME_OBJS) tools/align64.py
	$(PYTHON) tools/align64.py x86_64-w64-mingw32-objdump $(HL64_GAME_OBJS) > $(H64)/align.txt || \
		{ cat $(H64)/align.txt; exit 1; }
	@tail -1 $(H64)/align.txt
	@touch $@
$(H64)/bc_headless.exe: $(HL64_LINK_OBJS) $(H64)/align.ok
	$(LD64) -o $@ $(HL64_LINK_OBJS) $(LDFLAGS64) -Wl,-Map=$(H64)/bc_headless.map
	$(NM64) -n $@ > $(H64)/bc_headless.syms
	@head -3 $(H64)/link_report.txt

headless64: $(H64)/bc_headless.exe

# ---- game64 / dist64: bc.exe in 64 bits (as `game` / `dist`) ----
# RT64 for x86_64 from the same patched sources (rt64/build_rt64.sh ARCH=x86_64),
# the C++ host side (src/live/) with mingw-w64's x86_64 g++
G64 := build/game64
RT64_B64 := $(RT64_W)/build-x86_64
RT64_DLL64 := $(RT64_W)/dll-x86_64
RT64_LIBS64 := $(subst $(RT64_B)/,$(RT64_B64)/,$(RT64_LIBS))
CXX64 := x86_64-w64-mingw32-g++-posix
LIVE64_CXXFLAGS := $(subst -I$(RT64_B)/,-I$(RT64_B64)/,$(LIVE_CXXFLAGS))
G64_HOST_OBJS := $(filter-out $(H64)/host/headless_main.o,$(HL64_HOST_OBJS)) $(G64)/host/headless_main.o \
	$(G64)/live/live_rt64.o $(G64)/live/live_setup.o $(G64)/bc_res.o
$(G64)/rt64.stamp: rt64/build_rt64.sh $(wildcard rt64/patches/*.patch)
	@mkdir -p $(@D)
	ARCH=x86_64 TP=$(TP) JOBS=$${JOBS:-2} bash rt64/build_rt64.sh
	touch $@
$(G64)/host/headless_main.o: src/platform/headless_main.c src/platform/plat_host.h src/rdram.h Makefile port64.mk
	@mkdir -p $(@D)
	$(CC64) $(HL64_HOST_CFLAGS) -DPORT_LIVE -c -o $@ $<
$(G64)/live/%.o: src/live/%.cpp src/live/live_setup.h src/platform/plat_host.h src/rdram.h $(G)/version.h \
		$(G64)/rt64.stamp Makefile port64.mk
	@mkdir -p $(@D)
	$(CXX64) $(LIVE64_CXXFLAGS) -include $(G)/version.h -c -o $@ $<
$(G64)/bc_res.o: res/bc.rc $(G)/bc.ico $(G)/version.h
	@mkdir -p $(@D)
	x86_64-w64-mingw32-windres -I$(G) -Ires -O coff -o $@ $<
G64_LINK := $(HL64_GAME_OBJS) $(G64_HOST_OBJS) $(H64)/stubs.o $(H64)/copytab.o $(H64)/ptrtab.o $(H64)/abs_syms.o
$(G64)/bc.exe: $(G64_LINK) $(H64)/align.ok $(G64)/rt64.stamp
	$(CXX64) -o $@ $(G64_LINK) $(RT64_LIBS64) $(RT64_DLL64)/SDL2.dll $(RT64_DLL64)/libdxcompiler.a $(LIVE_SYSLIBS) \
		-static -static-libgcc -static-libstdc++ -mwindows $(LDFLAGS64) -Wl,-Map=$(G64)/bc.map
	$(NM64) -n $@ > $(G64)/bc.syms
	cp -u $(addprefix $(RT64_DLL64)/,$(RT64_DLLS)) $(G64)/
game64: $(G64)/bc.exe

DIST64_NAME := BlastCorps-port-$(BC_VERSION)-x64
DIST64 := build/dist/$(DIST64_NAME)
dist64: $(G64)/bc.exe dist/README.txt dist/third_party.sh
	rm -rf $(DIST64) $(DIST64).zip
	mkdir -p $(DIST64)
	cp $(G64)/bc.exe $(addprefix $(G64)/,$(RT64_DLLS)) $(DIST64)/
	x86_64-w64-mingw32-strip --strip-debug $(DIST64)/bc.exe
	sed -e 's/@VERSION@/$(BC_VERSION) (64-bit)/' -e 's/@BITS@/64-bit (x86_64)/' dist/README.txt | sed -e 's/$$/\r/' \
		> $(DIST64)/README.txt
	TP=$(TP) RT64_SRC=$(RT64_SRC) bash dist/third_party.sh | sed -e 's/$$/\r/' > $(DIST64)/THIRD_PARTY_LICENSES.txt
	cd build/dist && $(PYTHON) -m zipfile -c $(DIST64_NAME).zip $(DIST64_NAME)/
	@ls -l $(DIST64) $(DIST64).zip
.PHONY: game64 dist64

# ---- loadcheck64: the load layer through the 64-bit build (as `loadcheck`) ----
LC64 := build/lc64
LC64_CFLAGS := $(filter-out -msse2 -mfpmath=sse -Wno-builtin-declaration-mismatch,$(GAME_CFLAGS)) $(C64_COMMON) -g
LC64_GAME_OBJS := $(LC_FILES:%=$(LC64)/game/%.o)
LC64_HOST_OBJS := $(LC64)/host/rdram.o $(LC64)/host/inflate.o $(LC64)/host/loadcheck_main.o \
	$(LOAD_SRCS:src/load/%.c=$(LC64)/host/load/%.o) $(LC64)/host/swaptab.o
$(LC64)/game/%.raw.o: $(GAME_SRC)/%.c include/port_ultratypes.h include/port_n64ptr.h $(IR64_DEPS) Makefile port64.mk
	@mkdir -p $(@D)
	$(call cc64ir,$(LC64_CFLAGS))
$(LC64)/%.o $(LC64)/%.rec: $(LC64)/%.raw.o $(H64)/addrs.txt tools/coffpin.py
	$(PYTHON) tools/coffpin.py $(H64)/addrs.txt $< $(LC64)/$*.o $(LC64)/$*.rec
$(LC64)/host/%.o: src/%.c $(PORT_HDRS) Makefile port64.mk
	@mkdir -p $(@D)
	$(CC64) $(HL64_HOST_CFLAGS) -c -o $@ $<
$(LC64)/host/swaptab.o: $(H)/swaptab.c src/load/port_load.h
	@mkdir -p $(@D)
	$(CC64) $(HL64_HOST_CFLAGS) -c -o $@ $<
$(LC64)/abs_syms.o $(LC64)/stubs.c &: $(LC64_GAME_OBJS) $(LC64_HOST_OBJS) tools/gensyms.py tools/rdramobj.py
	cd $(ROOT) && NM=$(NM64) $(PYTHON) port/tools/gensyms.py link64 port/$(LC64) \
		$(addprefix port/,$(LC64_GAME_OBJS) $(LC64_HOST_OBJS))
$(LC64)/copytab.c: $(LC64_GAME_OBJS:.o=.rec) $(H64)/addrs.txt tools/gensyms.py
	$(PYTHON) tools/gensyms.py copytab64 $@ $(H64)/addrs.txt $(LC64_GAME_OBJS:.o=.rec)
$(LC64)/ptrtab.c: $(LC64_GAME_OBJS:.o=.rec) $(NM_ELFS) tools/gensyms.py
	cd $(ROOT) && $(PYTHON) port/tools/gensyms.py ptrtab port/$@ $(addprefix port/,$(LC64_GAME_OBJS:.o=.rec))
$(LC64)/%.o: $(LC64)/%.c src/rdram.h
	$(CC64) $(HL64_HOST_CFLAGS) -Isrc -w -c -o $@ $<
LC64_LINK := $(LC64_GAME_OBJS) $(LC64_HOST_OBJS) $(LC64)/stubs.o $(LC64)/copytab.o $(LC64)/ptrtab.o $(LC64)/abs_syms.o
$(LC64)/spike.exe: $(LC64_LINK)
	$(LD64) -o $@ $(LC64_LINK) $(LDFLAGS64)
loadcheck64: $(LC64)/spike.exe
	@if [ -n "$(LC_TRACE)" ]; then $(PYTHON) tools/widths.py facts $(LC64)/facts.txt $(LC_TRACE); fi
	$(LC64)/spike.exe '$(ROM_ARG)' $$([ -f $(LC64)/facts.txt ] && echo 56789 $(LC64)/facts.txt) \
		> $(LC64)/out_native.txt
	cd $(ROOT) && $(PYTHON) port/tools/loadref.py $(abspath $(ROM)) > port/$(LC64)/out_ref.txt
	@grep '^T9 total' $(LC64)/out_native.txt || true
	$(PYTHON) tools/cmpout.py $(LC64)/out_ref.txt $(LC64)/out_native.txt

# ---- verify64 / verify-levels64: the emulator comparisons with the 64-bit exe
# (tools/compare.py CMP_EXE; the native runs go to ...-native64 folders) ----
verify64:
	$(MAKE) -C $(ROOT) VERSION=$(VERSION) NON_MATCHING=1 nmrom BASEROM=$(abspath $(ROM)) > /dev/null
	$(MAKE) $(H64)/bc_headless.exe $(H)/typemap_all.txt
	$(MAKE) loadcheck64
	CMP_EXE=$(H64)/bc_headless.exe $(PYTHON) tools/compare.py verify --frames $(VERIFY_FRAMES) --cache $(CMP_CACHE) \
		$(VERIFY_ARGS)
verify-levels64:
	$(MAKE) -C $(ROOT) VERSION=$(VERSION) NON_MATCHING=1 nmrom BASEROM=$(abspath $(ROM)) > /dev/null
	$(MAKE) $(H64)/bc_headless.exe
	CMP_EXE=$(H64)/bc_headless.exe $(PYTHON) tools/compare.py verify-levels --cache $(CMP_CACHE) --jobs $(LEVEL_JOBS) \
		$(if $(LEVELS),--levels $(LEVELS)) $(VERIFY_LEVELS_ARGS)
.PHONY: loadcheck64 verify64 verify-levels64

# the pointer-width audit (tools/ptrcheck64.py: host-width pointers in N64
# records and pinned globals, casts to host-width pointer pointers, signed
# int -> pointer casts) and the record layouts i686 vs x86_64
# (tools/layout64.py); data/ptrcheck64_allow.txt lists what is host-only
P64_ALLOW := data/ptrcheck64_allow.txt
P64_GAME := $(HL_HD:%=$(GAME_SRC)/%.c) $(HL_FE:%=$(FE_SRC)/%.c)
P64_PLAT := $(HL_PLAT:%=src/platform/%.c)
P64_UL := $(HL_UL:%=$(H64)/ulsrc/%.c) $(HL_ULFE:%=$(H64)/ulsrc/%.c)
ptrcheck64: $(H64)/addrs.txt $(H64)/ulinc.stamp
	@rc=0; \
	$(PYTHON) tools/ptrcheck64.py $(H64)/addrs.txt $(P64_ALLOW) $(H64)/ptrcheck_game.txt -- \
		$(CC64) $(HL64_CFLAGS) -w -- $(P64_GAME) || rc=1; \
	$(PYTHON) tools/ptrcheck64.py $(H64)/addrs.txt $(P64_ALLOW) $(H64)/ptrcheck_plat.txt -- \
		$(CC64) $(HL64_PLAT_CFLAGS) -w -- $(P64_PLAT) || rc=1; \
	$(PYTHON) tools/ptrcheck64.py $(H64)/addrs.txt $(P64_ALLOW) $(H64)/ptrcheck_ul.txt -- \
		$(CC64) -I$(H)/le $(HL64_UL_CFLAGS) -w -- $(P64_UL) || rc=1; \
	$(PYTHON) tools/layout64.py $(P64_ALLOW) $(H64)/layout_game.txt -- $(CC64) $(HL64_CFLAGS) -- $(P64_GAME) || rc=1; \
	$(PYTHON) tools/layout64.py $(P64_ALLOW) $(H64)/layout_plat.txt -- $(CC64) $(HL64_PLAT_CFLAGS) -- $(P64_PLAT) \
		|| rc=1; \
	$(PYTHON) tools/layout64.py $(P64_ALLOW) $(H64)/layout_ul.txt -- $(CC64) -I$(H)/le $(HL64_UL_CFLAGS) -- $(P64_UL) \
		|| rc=1; \
	exit $$rc

.PHONY: headless64 ptrcheck64

# the signed-overflow census (tools/overflow_census.py; MSVC has no -fwrapv):
# a bc_headless built here with -fno-wrapv -fsanitize=signed-integer-overflow
# (build/hl64ubsan, tools/ubsan_minimal.c), run over the attract demos and every
# level; fails if any signed add/multiply overflows that the source doesn't
# wrap explicitly (PORT_WRAP_*, include/game/port.h).  OC_ARGS: e.g. --levels 0-9
OC := build/hl64ubsan
overflow-census:
	@mkdir -p $(OC)
	x86_64-w64-mingw32-gcc -O2 -c -o $(OC)/ubsan_minimal.o tools/ubsan_minimal.c
	$(MAKE) H64=$(OC) HL_EXTRA="-fno-wrapv -fsanitize=signed-integer-overflow -fsanitize-minimal-runtime" \
		LDFLAGS64="$(LDFLAGS64) $(OC)/ubsan_minimal.o" $(OC)/bc_headless.exe
	$(PYTHON) tools/overflow_census.py $(OC)/bc_headless.exe '$(ROM_ARG)' data/levels_input.txt $(OC)/runs $(OC_ARGS)
.PHONY: overflow-census
