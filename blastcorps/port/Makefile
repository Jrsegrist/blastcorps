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
	-D_LANGUAGE_C -D_FINALROM -DNON_MATCHING -DPORT_NATIVE -D_MIPS_SZLONG=32 -D_MIPS_SIM=1 \
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
LOAD_SRCS := src/load/swap.c src/load/port_assets.c
PORT_SRCS := src/rdram.c src/inflate.c src/$(MAIN).c $(LOAD_SRCS)
PORT_OBJS := $(PORT_SRCS:src/%.c=$(B)/port/%.o) $(B)/swaptab.o
PORT_HDRS := src/rdram.h src/load/port_load.h src/platform/port_dma.h

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
$(B)/swaptab.c: $(B)/typemap.txt data/image_widths.txt tools/swaptab.py $(NM_ELFS)
	cd $(ROOT) && $(PYTHON) port/tools/swaptab.py port/$(B)/typemap.txt port/data/image_widths.txt port/$@ \
		port/$(B)/swaptab_report.txt > port/$(B)/swaptab_coverage.txt
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

ucode:
	cd $(ROOT) && $(PYTHON) port/tools/ucode_id.py build

clean:
	rm -rf $(B)

.PHONY: all check clean linkall typemap ucode
.SECONDARY:
