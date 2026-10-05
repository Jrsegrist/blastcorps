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
| `undefined_*.txt` | `name = addr;` (overrides objects) | functions (`func_*`, aliases `a = b;`, all of `undefined_funcs*`) become `PROVIDE(name = addr);`, so a moved function's own definition wins; data assignments stay hard |
| `all` | sha1 verify | link only |

Data assignments must stay hard because some data symbols are pinned in
`undefined_syms` *and* defined in C (hd.c's u64 `D_80364A88/90/98` sit outside
its modelled `.bss`). With a `PROVIDE()` the C definition would win in
`build_nm` and the variable would move (D_80364A98 landed at 0x80312D88, on
top of D_80312D80), while the hand asm still used 0x80364A98: two homes for one
variable. `python3 tools_port/nm_symaudit.py` compares the symbol tables of
`build/` and `build_nm/` and lists every non-text symbol whose address
differs; `runchecks -b`/`-B` runs it after building and stops if anything moved.

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
python3 tools_port/eqcheck.py FUNC [options]     # from the repo dir, venv active
bash tools_port/eq.sh FUNC [options]             # same, from anywhere: activates ../.env and cds to the repo
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

Heap bytes are labelled by block (`heap0 (0x80B00000)`, see `ptr` below).
Loads from data tables that live inside the text range (for example
`D_802C23B4` in the 7D9D0 blob) are listed with an `(in text)` suffix.
Contiguous bytes with no nearby symbol are merged into one `(unnamed) 0x...`
range, and a run of more than four back-to-back symbol groups (a scan running
across many globals) prints as one `D_A .. D_B` line, so a runaway scan of an
unterminated table is a single line, not thousands.

### Inputs

All registers (GPRs, FPRs, hi/lo) and the caller's 0x100-byte frame at `sp`
get identical pseudo-random poison in both runs. `sp` is 0x80E00000 and `ra`
is a sentinel. On top of that you can set:

* `--arg REG=SPEC` takes any register (`a0`..`a3`, `f12`, `f14`, `t0`, `s1`, and so on), or `--arg sp+0x10=SPEC` for stack arguments.
* `--mem TARGET[:SIZE]=FILL` sets memory contents. TARGET is `SYM`, `SYM+0x10`, a raw address, or relative to an argument or heap block: `@a0`, `@a0+0x40`, `@heap1+0x10` (see below). Symbols resolve separately in each build. An unknown symbol is an error naming the symbol and the build.
* **Sizing.** Without `:SIZE` a value fill writes a word (4 bytes). For `--mem SYM=...` (no offset) whose ELF symbol has a size of 1 or 2 (C-defined `u8`/`s16` globals) the write is sized from the symbol, with a `note:`. Absolute `D_` symbols from `undefined_syms*.txt` have no size; then, if the next symbol starts less than 4 bytes after the target, you get a `warning:` and should write `SYM:1=` or `SYM:2=` yourself. A word written over a byte global silently clobbers its neighbours.
* `--mem TARGET*STRIDE:COUNT[/W]=FILL` fills the same W-byte field (default 4) in COUNT records STRIDE bytes apart, each record drawn separately: `--mem D_803FB8B8+8*0x14:25/2=choice:0,0x7FFF` sets the halfword at +8 of all 25 entries of a 0x14-byte table.
* `--heap SIZE[=FILL]` allocates a scratch-heap block without putting its address in any register (FILL defaults to `rand`; `zero`, `words:..` etc. work). Use it for memory reached only through a global or a field: `--heap 0x300 --mem D_80358074=rel:heap0`. `--heap` blocks are numbered first (`heap0`, `heap1`, ...), before `--arg ptr` blocks.

| SPEC / FILL | meaning |
|---|---|
| `0x1234` | constant |
| `int` / `int:LO:HI` | random int (biased toward small values) or uniform in a range |
| `choice:0,1,5` | one of these values; `choice:0x50*3,0x5A` weights a value (picked 3x as often) |
| `float[:LO:HI]` | random single in an FPR (`f12=float`) |
| `fbits[:LO:HI]` | float bits in an integer register or stack slot (o32 float args after an int arg) |
| `ptr[:SIZE]` / `ptrz[:SIZE]` | pointer to a fresh scratch-heap block (random or zero bytes, default 0x200) |
| `sym:NAME` / `sym:NAME+0x10` | the address of NAME (in a register too, resolved per build) |
| `rel:BASE+TERM+..` | an address relative to another value: see "Pointer-relative values" |
| `rand`, `zero` | (`--mem`) SIZE random or zero bytes |
| `words:a,b,c` / `halves:a,b` / `bytes:a,b` | (`--mem`) each word, halfword or byte chosen from a pool (weights `v*N` allowed), which is good for flags, indices, and NULL vs non-NULL pointers |
| `floats[:LO:HI]` | (`--mem`) SIZE/4 random floats |
| `onehot:STRIDE[@OFF][/W]:POOL:FILL` | (`--mem`, needs SIZE) fill with FILL, then put one value from POOL into exactly one slot: see "One special slot" |

#### Order of application

Registers and the caller's frame are poisoned first. Then the `--heap` blocks
are allocated and filled. Then every `--arg` is
generated, left to right except that `rel:` args come after all the others; a
`ptr` block's random contents are laid down at this point. Then every `--mem`,
left to right. Later writes win, so a `--mem` into a `ptr` block (or into the
stack-argument area) overrides the block's random bytes. (Before Oct 2026 the
order was the reverse and `ptr` data silently overwrote `--mem` specs.)

#### Pointer-relative values

Scratch-heap blocks are numbered in allocation order: `heap0`, `heap1`, ...
(`--heap` blocks first, then `--arg ... ptr` specs, then `ptr` fills in
`--mem`). The first block is always 0x80B00000. `heap` alone is 0x80B00000.

* `--mem @a0+0x40=0x50` writes into the memory an argument points at. `@NAME`
  is an `--arg` register (`a0`..), `heapN`, or `heap`.
* `--arg a1=rel:a0+0x60*int:0:5` makes a value relative to another one.
  BASE is an `--arg` register, `heap`/`heapN`, a symbol (`NAME` or `sym:NAME`), or a number. Each
  `+TERM` is a constant (`+0x10`, `+-4`), any value spec (`+choice:0,0x60,0x5A0`),
  or `K*SPEC`, K times a random value (`+0x60*int:0:16`, `+0x14*choice:0,24`).
  The same works as a `--mem` fill, which stores a pointer: `--mem D_80358074=rel:heap0+0x20`.

```
# a1 = a0 + 0..16 records of 0x60 bytes, inside a 0x600-byte block
eqcheck.py func_802CEA68 --arg a0=ptr:0x600 --arg a1=rel:a0+0x60*int:0:16 --ret void
# a global pointing at a heap block, and fields inside that block
eqcheck.py func_802C1EE0 --arg a0=int:1:20 --mem D_80358074=ptr:0x300 \
    --mem @heap0+0x74=choice:0x100*2,0x102,0 --mem @heap0+0x100:0x200=halves:4,6,8,0x10
```

#### One special slot

`onehot:STRIDE[@OFF][/W]:POOL:FILL` fills SIZE bytes with FILL (any fill;
a plain constant repeats as a word), then picks one slot of the
SIZE/STRIDE-slot array and writes a value from POOL (comma list, weights
allowed) at byte OFF of that slot (default 0), W bytes wide (1, 2 or 4,
default 4). The slot is the first one 30% of the time, the last one 30%, and a
uniformly random one otherwise, so one run covers first/last/middle. Examples
for a 25 x 0x14 table whose word 0 is an id (-1 = free):

```
--mem D_803FB8B8:500=onehot:0x14:-1:words:0,1,2          # exactly one free slot
--mem D_803FB8B8:500=onehot:0x14:0x100:words:-1,0,1      # the searched id in exactly one slot
--mem D_803FB8B8:500=onehot:0x14@8/2:0x7FFF:rand         # one slot's halfword at +8 = 0x7FFF
```

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
* **Static helpers are followed automatically.** A callee that exists only in the new build (no symbol of that name in the reference build, typically a `static` helper a rewrite introduced) is part of the rewrite, so it always runs for real instead of showing up as an extra call. The run prints `note: following NAME, which exists only in build_nm`.
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

## 4. Regression suite

Every rewrite's eqcheck runs are checked in, so any later change (a struct
refactor, a shared header, an eqcheck change) can be re-verified in one go.

**Spec format.** One file per source file, `tools_port/checks/<FILE>.txt`
(for `src.us.v11/hd_code/<FILE>.c`). Each line is one eqcheck run:

```
# comment
func_802CE880: -n 300 --ret void --mem D_803FB8B8:500=words:-1,0,1,2 --arg a0=choice:-1,0,0x100
func_802CE880: -n 200 --ret void --seed 4 --mem D_803FB8B8:500=onehot:0x14:-1:words:0,1,2 \
    --arg a0=choice:0,3,0x100
```

A function can have any number of lines (a random run plus boundary runs);
give each line of the same function its own `--seed`. Arguments are split
shell-style (quotes work) but nothing is expanded, so write values out instead
of using `$VARS`. A trailing `\` continues a line.

**Running.**

```bash
bash tools_port/runchecks.sh            # everything, plus the coverage check
bash tools_port/runchecks.sh -b         # make NON_MATCHING=1 first
bash tools_port/runchecks.sh -B         # matching build (checks both OK lines) + NON_MATCHING first
bash tools_port/runchecks.sh 8A080 func_802A5510   # filter: check file, function, or substring
bash tools_port/runchecks.sh -q 20      # quick: cap every run at 20 trials
bash tools_port/runchecks.sh --list 60D50          # show the selected command lines
bash tools_port/runchecks.sh --coverage-only
bash tools_port/runchecks.sh -b --changed          # only files whose .c or check file differs from main
bash tools_port/runchecks.sh --changed --since HEAD~3
```

`--changed` selects check files whose `src.us.v11/hd_code/<FILE>.c` or
`tools_port/checks/<FILE>.txt` differs from the merge base of `--since`
(default `main`) and HEAD, counting committed, staged, unstaged and untracked
changes, and reports any rewrite in those files that has no check line. It doesn't notice
changes to shared headers or to eqcheck itself; run the whole suite for those.

Runs go in parallel (`-j`, default min(cores, 8)). The output is a table with one row per run (file,
function, line, PASS/FAIL/ERROR, trials, time). A failing run shows the
eqcheck failure lines and a ready-to-paste `bash tools_port/eq.sh ...` rerun
command. A run in which every trial faulted counts as a FAIL. The exit status
is nonzero on any failure.

**Coverage.** Without a filter the runner also lists every function whose
`#pragma GLOBAL_ASM` sits in the `#else` branch of an `#ifdef NON_MATCHING`
block in `src.us.v11/hd_code/*.c` and reports any with no check line
(`MISSING`, which makes the exit status nonzero). It also notes check lines for
functions that have no rewrite.

Current suite (Oct 2026): 61 rewritten functions, 148 runs, all PASS, about
70 s wall at `-j4` and 58 s at `-j8` (was 637 s at `-j4` before the
restore/diff speedup, see section 6).

## 5. Writing checks for a new rewrite

1. **`--explore` first.** Note every global, heap and stack-argument byte read
   and every call. Re-run it with your inputs to confirm they're the ones read.
2. **One random run, 200-400 trials**, randomizing every input the function
   reads, with pools that reach every branch (`choice:`, `words:`, weights).
   **Pre-fill the outputs with `rand` too** (every global, field or buffer the
   function writes): the diff only sees bytes that *change*, so a store of 0
   into memory that is already 0 (the default bss) is invisible, and a rewrite
   that drops it passes.
3. **Size byte and halfword globals.** An unsized `--mem SYM=VAL` writes 4
   bytes. eqcheck sizes it from the ELF when the symbol has a size, and warns
   when the next symbol is closer than 4 bytes; otherwise write `SYM:1=`/`SYM:2=`
   yourself, or the word spills into the neighbouring globals.
4. **Explicit boundary runs** (each with its own `--seed`): 0, -1, NULL;
   sentinel values and their neighbours; table full and table empty; only the
   first or only the last slot special (`onehot:`); 0x7FFF / 0x8000 / 0xFFFF
   for halfwords, 0x7FFFFFFF / 0x80000000 for words; signed vs unsigned shifts
   (negative inputs). Random pools almost never hit "exactly one slot special
   at the boundary" (the pilot's off-by-one loop bound passed 600 random trials).
   For a field in every record of a table use the stride form
   (`--mem TBL+8*0x14:25/2=choice:..`); for memory reached only through a
   global pointer use `--heap SIZE` plus `--mem PTR=rel:heap0`.
5. **Terminate scanned tables.** A table walked until a sentinel must have one
   in `--mem` (with the default zero bss the walk runs off through megabytes).
   Loops with `!=` bounds need inputs that respect the precondition.
6. **`--ret void` for void functions** (otherwise leftover v0 is compared);
   use the real width (`--ret int` when the asm returns the full 32-bit value).
7. **`--sig NAME=a0,a1`** for callees that take fewer than 4 arguments, so
   garbage in unused argument registers isn't compared.
8. **Static helpers are fine.** A rewrite may split work into `static`
   functions: callees that exist only in `build_nm` are run, not stubbed, so
   they don't show up as extra calls. (Helpers must not share a name with any
   function in the matching build, or they'll be stubbed and compared as calls.)
9. **Mutation spot-check.** Break the C once on purpose (loop bound, a
   dropped check, `<` for `<=`, a dropped store of 0), rebuild NON_MATCHING,
   confirm at least one check line FAILs, then revert and rebuild.
10. Add the lines to `tools_port/checks/<FILE>.txt` and run
   `bash tools_port/runchecks.sh -b <FILE>` (or `-b --changed`), then the whole
   suite before you commit.

## 6. Demo results (Oct 2026)

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

A trial (both builds) takes about 5-10 ms for a small function, and a run
starts in about 0.4 s. Guest RAM is mapped from host buffers
(`mem_map_ptr`), so after each run the harness memcmps the writable regions
against the start-of-run snapshot in place (64 KB chunks, then 4 KB pages,
then bytes only inside changed pages) and before the next run copies back only
the pages that changed or held inputs. The parsed ELF symbol tables are cached
in `<build dir>/.eqcheck_cache.<version>.pickle`, keyed by the ELFs' size and
mtime. (Before Oct 2026 every trial copied and compared about 6 MB through
`mem_read`/`mem_write`: about 33 ms per trial single-threaded and much more
under `-j4`; the 148-run suite went from 637 s to 69 s at `-j4`.)

## 7. Known limitations

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
