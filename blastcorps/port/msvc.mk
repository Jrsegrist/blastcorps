# ---------------------------------------------------------------------------
# The MSVC build (port/CMakeLists.txt) from WSL: its inputs, the build, and
# the same checks as the other builds run against its exes.  See README.md
# ("Build on Windows with Visual Studio").  Included by Makefile.
#
#   make -C port inputs     build/inputs/: what the CMake build takes from the
#                           NON_MATCHING build (addresses, symbol kinds, pointer
#                           slots, types, link maps); Python's standard library
#                           is all the Windows side needs then
#   make -C port msvc       build the MSVC exes (tools/msvc_build.sh: a copy of
#                           the sources in MSVC_DIR, CMake + Ninja in Visual
#                           Studio's x64 environment) -> MSVC_DIR/build-Release
#   make -C port verify-msvc / verify-levels-msvc / loadcheck-msvc / dist-msvc
#
# MSVC_DIR: a folder on a Windows drive (default %LOCALAPPDATA%\blastcorps-msvc);
# MSVC_CONFIG: Release (default) or Debug; MSVC_PYTHON: a Windows python.exe if
# CMake can't find one.
INPUTS := build/inputs
inputs: $(INPUTS)/stamp
$(INPUTS)/stamp: $(NM_ELFS) $(H)/typemap.txt tools/gensyms.py
	@mkdir -p $(INPUTS)
	cd $(ROOT) && $(PYTHON) port/tools/gensyms.py inputs port/$(INPUTS)
	cp $(H)/typemap.txt $(INPUTS)/
	cp $(ROOT)/build_nm/hd_code.$(VERSION).map $(ROOT)/build_nm/hd_front_end.$(VERSION).map $(INPUTS)/
	touch $@

MSVC_DIR ?= $(shell d=$$(cd /mnt/c && cmd.exe /c 'echo %LOCALAPPDATA%' 2>/dev/null | tr -d '\r'); \
	[ -n "$$d" ] && echo "$$(wslpath "$$d")/blastcorps-msvc")
MSVC_CONFIG ?= Release
MSVC_B = $(MSVC_DIR)/build-$(MSVC_CONFIG)
MSVC_TARGETS ?=
msvc: inputs
	@[ -n "$(MSVC_DIR)" ] || { echo "msvc: set MSVC_DIR (a folder on a Windows drive)"; exit 1; }
	MSVC_PYTHON='$(MSVC_PYTHON)' BC_VERSION=$(BC_VERSION) bash tools/msvc_build.sh '$(MSVC_DIR)' $(MSVC_CONFIG) \
		$(MSVC_TARGETS)

# loadcheck through the MSVC exe (as `loadcheck`)
LCM := build/lcmsvc
loadcheck-msvc:
	$(MAKE) msvc MSVC_TARGETS=bc_loadcheck
	@mkdir -p $(LCM)
	@if [ -n "$(LC_TRACE)" ]; then $(PYTHON) tools/widths.py facts $(LCM)/facts.txt $(LC_TRACE); fi
	'$(MSVC_B)/bc_loadcheck.exe' '$(ROM_ARG)' $$([ -f $(LCM)/facts.txt ] && echo 56789 \
		$$(wslpath -w $(LCM)/facts.txt)) > $(LCM)/out_native.txt
	cd $(ROOT) && $(PYTHON) port/tools/loadref.py $(abspath $(ROM)) > port/$(LCM)/out_ref.txt
	@grep '^T9 total' $(LCM)/out_native.txt || true
	$(PYTHON) tools/cmpout.py $(LCM)/out_ref.txt $(LCM)/out_native.txt

# the emulator comparisons with the MSVC exe (tools/compare.py CMP_EXE; the
# native runs go to ...-nativemsvc folders, CMP_TAG)
verify-msvc:
	$(MAKE) -C $(ROOT) VERSION=$(VERSION) NON_MATCHING=1 nmrom BASEROM=$(abspath $(ROM)) > /dev/null
	$(MAKE) msvc MSVC_TARGETS="bc_headless bc_loadcheck"
	$(MAKE) $(H)/typemap_all.txt
	$(MAKE) loadcheck-msvc
	CMP_EXE='$(MSVC_B)/bc_headless.exe' CMP_TAG=msvc $(PYTHON) tools/compare.py verify --frames $(VERIFY_FRAMES) \
		--cache $(CMP_CACHE) $(VERIFY_ARGS)
verify-levels-msvc:
	$(MAKE) -C $(ROOT) VERSION=$(VERSION) NON_MATCHING=1 nmrom BASEROM=$(abspath $(ROM)) > /dev/null
	$(MAKE) msvc MSVC_TARGETS=bc_headless
	CMP_EXE='$(MSVC_B)/bc_headless.exe' CMP_TAG=msvc $(PYTHON) tools/compare.py verify-levels --cache $(CMP_CACHE) \
		--jobs $(LEVEL_JOBS) $(if $(LEVELS),--levels $(LEVELS)) $(VERIFY_LEVELS_ARGS)

.PHONY: inputs msvc loadcheck-msvc verify-msvc verify-levels-msvc
