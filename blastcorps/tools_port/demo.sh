#!/bin/bash
# Equivalence-harness demo / smoke test. Run from the repo dir (blastcorps/)
# with the venv active, after both builds:
#   make VERSION=us.v11 -j8 && make VERSION=us.v11 NON_MATCHING=1 -j8
set -e
E="python3 tools_port/eqcheck.py"

echo "== already-matched C functions, matching build against itself"
$E func_80275DA4 --new build -n 50 --arg a0=ptrz:0x100 --arg a1=choice:0,1 --ret ptr
$E func_8027684C --new build -n 50 --mem D_8036C7A0:40=words:0,1,2 --mem D_8036C794=int --ret void

echo "== already-matched C functions, matching vs NON_MATCHING build (same C, relocated)"
$E func_80275DA4 -n 100 --arg a0=ptrz:0x100 --arg a1=choice:0,1 --ret ptr
$E func_802768A8 -n 100 --mem D_8036C7A0:40=words:0,1,2 --mem D_8036C790=choice:0,1,2
$E func_8027684C -n 100 --mem D_8036C7A0:40=words:0,1,2 --mem D_8036C794=int --ret void
$E func_80276D1C -n 100 --follow guMtxL2F --arg a0=ptr:0x40 \
   --arg a1=fbits --arg a2=fbits --arg a3=fbits --arg sp+0x10=fbits \
   --arg sp+0x14=ptrz:4 --arg sp+0x18=ptrz:4 --arg sp+0x1C=ptrz:4 --arg sp+0x20=ptrz:4 --ret void

echo "== hand-written asm vs its NON_MATCHING C rewrite"
$E func_802CE840 -n 50 --ret void
$E func_802C8AB0 -n 200 --arg a0=int --ret void
