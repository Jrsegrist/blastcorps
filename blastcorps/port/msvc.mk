# ---------------------------------------------------------------------------
# The MSVC build (port/CMakeLists.txt) from WSL: its inputs, and the same
# checks as the other builds run against its exes.  See README.md ("Build on
# Windows with Visual Studio").  Included by Makefile.
#
#   make -C port inputs     build/inputs/: what the CMake build takes from the
#                           NON_MATCHING build (addresses, symbol kinds, pointer
#                           slots, types, link maps); Python's standard library
#                           is all the Windows side needs then
INPUTS := build/inputs
inputs: $(INPUTS)/stamp
$(INPUTS)/stamp: $(NM_ELFS) $(H)/typemap.txt tools/gensyms.py
	@mkdir -p $(INPUTS)
	cd $(ROOT) && $(PYTHON) port/tools/gensyms.py inputs port/$(INPUTS)
	cp $(H)/typemap.txt $(INPUTS)/
	cp $(ROOT)/build_nm/hd_code.$(VERSION).map $(ROOT)/build_nm/hd_front_end.$(VERSION).map $(INPUTS)/
	touch $@
.PHONY: inputs
