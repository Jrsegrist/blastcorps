# tools_port: NON_MATCHING builds and the equivalence harness

The native port needs real C for the roughly 660 hand-written asm functions in
hd_code (56040 through 8DDB0). That C can't byte-match, so it is checked for
*functional* equivalence instead. Two pieces support this:

* `make NON_MATCHING=1` builds `#ifdef NON_MATCHING` rewrites into `build_nm/`.
* `eqcheck.py` runs a function from both builds in a MIPS emulator on
  identical random inputs and diffs the results.

## 1. Writing a rewrite

```c
#ifdef NON_MATCHING
void func_802CE840(void) {
    s32 i;

    for (i = 0; i < 25; i++) {
        *(s32 *) (D_803FB8B8 + i * 0x14) = -1;
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/hd_code/8A080/func_802CE840.s")
#endif
```

Put any externs or prototypes the rewrite needs inside the `#ifdef`, so the
matching build sees exactly the same source as before. asm-processor already
handles a pragma sitting inside `#else`. Worked examples:
`src.us.v11/hd_code/8A080.c` (func_802CE840) and `83910.c` (func_802C8AB0).

## 2. Building

```bash
cd ~/blastcorps/blastcorps && source ../.env/bin/activate
make VERSION=us.v11 -j8                  # matching: must still print both ": OK" lines
make VERSION=us.v11 NON_MATCHING=1 -j8   # -> build_nm/{hd_code,init}.us.v11.elf, no checksum
```

What `NON_MATCHING=1` changes (Makefile plus `tools_port/nm_ldscript.py`):

| | matching (`build/`) | NON_MATCHING (`build_nm/`) |
|---|---|---|
| CFLAGS | | `-DNON_MATCHING` |
| hd_code `.text` VRAM | 0x802447C0 | `NM_TEXT_VRAM` (default 0x80800000) |
| `.hd_code_data` | 0x802E8BD0 | same address, and every `*_bin` blob is pinned to its original address |
| sections nothing places (e.g. new `.rodata` in a hand-asm file) | discarded, so the link fails | `.nm_extra` at `NM_EXTRA_VRAM` (0x80A00000) |
| `undefined_*.txt` | `name = addr;` (overrides objects) | `PROVIDE(name = addr);`, so a moved function's own definition wins |
| `all` | sha1 verify | link only |

Why the text moves: code ends exactly where the data segment starts, and most
data symbols are absolute `D_xxxxxxxx = 0x...` definitions, so data can't move.
Any rewrite that grows would run into it. Moving the text clear of the data
means data and `.bss` keep their original addresses, and only code addresses
change. The harness maps those by symbol name. The NM ELF is a verification
artifact, not a bootable ROM: function pointers stored in the raw data blobs
still hold the original addresses.

make doesn't track flag changes. Because `build_nm/` is a separate tree,
switching modes never clobbers the matching objects.

## 3. Running the harness

Setup is done once and is already installed: `pip install unicorn pyelftools` into `~/blastcorps/.env`.

```bash
python3 tools_port/eqcheck.py FUNC [options]
```

By default it compares `build/` (the original asm) against `build_nm/` (your
rewrite), runs 200 trials, and stops at the first failing trial.

**Start with `--explore`.** It runs the reference once and prints which
globals, heap, or stack bytes the function read and wrote, plus the calls it
made with their arguments. Use that to decide which inputs to randomize:

```
$ python3 tools_port/eqcheck.py func_8027684C --explore --mem D_8036C7A0:40=words:0,1,2 --mem D_8036C794=int
reads:
  D_8036C794               0x8036C794..0x8036C798 (4 bytes)
  D_8036C7A0               0x8036C7A0..0x8036C7AC (12 bytes)
writes:
  D_8036C7A0               0x8036C7A8..0x8036C7AC (4 bytes)
```

### Inputs

All registers (GPRs, FPRs, hi/lo) and the caller's 0x100-byte frame at `sp`
get identical pseudo-random poison in both runs. `sp` is 0x80E00000 and `ra`
is a sentinel. On top of that you can set:

* `--arg REG=SPEC` takes any register (`a0`..`a3`, `f12`, `f14`, `t0`, `s1`, and so on), or `--arg sp+0x10=SPEC` for stack arguments.
* `--mem TARGET[:SIZE]=FILL` sets a global's contents. TARGET is `SYM`, `SYM+0x10`, or a raw address. Symbols resolve separately in each build.

| SPEC / FILL | meaning |
|---|---|
| `0x1234` | constant |
| `int` / `int:LO:HI` | random int (biased toward small values) or uniform in a range |
| `choice:0,1,5` | one of these values |
| `float[:LO:HI]` | random single in an FPR (`f12=float`) |
| `fbits[:LO:HI]` | float bits in an integer register or stack slot (o32 float args after an int arg) |
| `ptr[:SIZE]` / `ptrz[:SIZE]` | pointer to a fresh scratch-heap block (random or zero bytes, default 0x200) |
| `sym:NAME` | (`--mem` only) store the address of NAME |
| `rand`, `zero` | (`--mem`) SIZE random or zero bytes |
| `words:a,b,c` / `halves:a,b` | (`--mem`) each word or halfword chosen from a pool, which is good for flags, indices, and NULL vs non-NULL pointers |
| `floats[:LO:HI]` | (`--mem`) SIZE/4 random floats |

Choose pools that reach every branch. In the demo, a broken
`func_802768A8` (with its NULL check dropped) **passes** when
`D_8036C790=choice:1,2` and fails at once with `choice:0,1,2`. The harness
only proves equivalence on the inputs you give it.

### Calls

Every call to another function is **stubbed** by default. The harness records
the callee's name and its argument registers (`a0`..`a3` unless you override
them), puts a canned value in `v0`, `v1`, and `f0`, and returns. The canned
value is a deterministic hash of the seed, the trial, the callee, and the call
number, so both builds get the same values. The ordered list of calls is
compared between the builds.

* `--follow NAME[,NAME]` runs that callee for real, using each build's own version of it. `--follow-all` runs every callee.
* `--sig NAME=a0,a1` sets which registers count as NAME's arguments. Use it when a callee takes fewer than 4 arguments and the two versions leave different garbage in the unused ones. `--sig '*=a0'` changes the default. Non-ABI callees can list t-registers (`--sig func_802ABD54=a0,a1,a2,a3,t3,t4,t5`).
* `--stub-ret NAME=VALUE|rand|ptr:SIZE` sets what a stubbed callee returns. `ptr` hands out deterministic heap blocks, which is useful for allocators.

Calls are recognized by control reaching a function's entry address, so
`jal`, `jalr`, and tail `j`/`b` all count. A target outside any loaded code
(another overlay, or a NULL function pointer) becomes `sub_XXXXXXXX` and is
stubbed.

### What is compared

1. **Outcome.** Did both runs return? A run that hits the same unmapped address or CPU exception in both builds counts as equivalent and is reported in the summary. A timeout in either build is a failure.
2. **Return registers.** Set with `--ret int` (default `v0`), `ptr`, `void`, `u64` (`v0`,`v1`), `float` (`f0`), `double` (`f0`,`f1`), or `regs:v0,t0,...` for non-ABI outputs.
3. **Callee-saved registers** (`s0`-`s7`, `fp`, `sp`, `gp`, `f20`-`f31`). This catches hand asm that returns values in s-registers. Use `--ignore-reg R` or `--no-saved-check` to relax it.
4. **Memory.** Every byte that differs from the start-of-run state in either run: game RAM (data, bss, globals), heap, caller frame, NM-only sections, and the MMIO register blocks. Bytes below the entry `sp` are the callee's private frame and are ignored, as are the 16 arg-home bytes at `0(sp)` unless you pass `--check-home`. Addresses that exist only in the new build are translated by symbol.
5. **Call sequence**, plus the MMIO access sequence with `--mmio-log`.

Values that are code addresses (function pointers in arguments or stored to
memory) are matched by symbol name, because NM text sits at a different address.

A failure prints the trial number, the generated inputs, and the first
difference, for example:

```
FAIL trial 11 (seed 0)
  inputs: D_8036C7A0 = words 1,2,2,1,2,1,1,1,1,0; D_8036C794 = 1
  memory: 1 byte(s) differ; first at 0x8036C7C7 D_8036C7A0+0x27: build=01 build_nm=00 (word @0x8036C7C4: 00000001 vs 00000000)
```

To reproduce a failure, rerun with the same `--seed` (and `-n` covering that
trial). `-v` lists more differing bytes and the event lists. `--trace` prints
every basic block both builds execute.

### Other options

`-n N` sets the number of trials. `--seed S` sets the seed. `--max-fail K` keeps going after the first failure. `--ref DIR` and `--new DIR` pick the builds (use `--new build` to compare a build with itself). `--max-insns` (default 2M) and `--timeout` (default 10 s per run) bound each run. `--mmio 0xA4400010=0x200` presets an MMIO word. `--ignore-new-only` skips writes to NM-only memory.

`tools_port/demo.sh` runs the demo set below and works as a smoke test.

## 4. Demo results (Oct 2026)

| function | kind | comparison | result |
|---|---|---|---|
| func_80275DA4 (GBI setup, writes a display list) | matched C | build vs build / vs build_nm | PASS 50 / 100 |
| func_802768A8 (search a 10-entry table) | matched C | vs build_nm | PASS 100 |
| func_8027684C (insert into a free slot) | matched C | build vs build / vs build_nm | PASS 50 / 100 |
| func_80276D1C (matrix × vec4, `--follow guMtxL2F`) | matched C | vs build_nm | PASS 100 |
| func_8027684C, loop bound changed to 9 | broken | vs build_nm | FAIL trial 11: `D_8036C7A0+0x27` differs |
| func_802768A8, NULL check dropped | broken | vs build_nm | PASS with pool {1,2}; FAIL trial 1 with {0,1,2}: `$v0` 0 vs 1 |
| func_80275DA4, wrong geometry mode | broken | vs build_nm | FAIL trial 0: `heap+0xE` 02 vs 00 |
| func_802CE840 (fills a 25-entry table) | hand asm, C rewrite | vs build_nm | PASS 50 |
| func_802C8AB0 (sets 2 globals, calls func_802C4310(a0, 0x72)) | hand asm, C rewrite | vs build_nm | PASS 200 |

A trial takes about 25 ms, most of it restoring and diffing about 6 MB of
memory, so 200 trials run in about 5 s.

## 5. Known limitations

* **Equivalence is only as good as the inputs.** Randomize every global the function reads (`--explore` lists them), and use value pools that reach the edge cases: NULL, 0, negative values, counts at their bounds.
* **No hardware.** MMIO (`0xA4xxxxxx`: SP, DP, MI, VI, AI, PI, RI, SI) is plain memory. A write then a read returns the written value, nothing has side effects, DMA never happens, and status registers read whatever you preset with `--mmio`, otherwise 0. A loop polling a busy bit therefore exits at once or spins until `--timeout`. RSP and RDP code (microcode, `osSpTask*`) and cartridge or PIF space (unmapped, so access faults) can't be tested this way. Final MMIO state is compared; the order of accesses is compared only with `--mmio-log`.
* **Interrupts, threads, and the OS** don't exist. Stub calls into libultra (the default), or follow only pure ones such as `guMtxL2F`.
* **FPU.** Runs as R4000 with Status.FR=0, as IDO's o32 code expects: doubles live in even/odd pairs. The FCSR starts at 0 (round to nearest, no traps). The harness compares raw bits, so a different NaN payload or a different but legitimate rounding order (for example `a*b+c` evaluated in another order) shows up as a difference. Inspect those by hand. Results under flush-to-zero or unimplemented-operation exceptions follow QEMU, not the VR4300.
* **64-bit registers.** The CPU runs in 64-bit mode, because the hand asm uses `sd`/`ld`/`dsll`. Inputs are sign-extended 32-bit values. Comparisons use the low 32 bits, so functions passing true 64-bit values in a single register aren't fully checked.
* **Non-ABI functions.** Asm that takes inputs in t-registers or returns values in s- or t-registers can be described with `--arg t3=...`, `--ret regs:...` and `--sig`. C can't reproduce such a convention, though. When a rewrite changes it, rewrite the callers too and test at a boundary that is ABI-clean again (the caller, with `--follow` on the callee).
* **Call detection is entry-address based.** Asm that falls through into the next function, or jumps into the middle of another function, isn't seen as a call. The fall-through target becomes a recorded call only in the rewrite, which looks like a spurious difference. Fix it with `--follow` on that target.
* **Data blobs keep original code addresses.** Jump tables and function-pointer tables inside the raw `.bin` blobs, and functions that live *inside* a blob (such as func_802C4310 in `7D9D0_data`), point at the original text. The NM machine is pre-filled with the original text at its old address, so those paths still execute (the original code). Function entries reached that way are translated by name, and other blocks print a `note:`.
* **Infinite loops.** These are caught by `--max-insns` and `--timeout`. They are reported as a failure if only one build loops, and as "can't compare" if both do.
* **unicorn bug (worked around).** In unicorn 2.1, a memory hook that fires for a load or store in a branch delay slot corrupts MIPS execution (RI exception at the next jump). That is why writes are found by diffing memory rather than with hooks, and why `--explore` and `--mmio-log` decode loads and stores in a per-instruction code hook. Don't add `UC_HOOK_MEM_READ`/`UC_HOOK_MEM_WRITE` hooks.
* Only `hd_code` and `init` are loaded. Calls into other overlays are stubbed as `sub_XXXXXXXX`, and reading their data faults or returns 0.
