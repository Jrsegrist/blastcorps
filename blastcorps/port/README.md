# port/: Windows build (stage 0 spike)

A 32-bit (i686) Windows exe that maps the N64's 8 MB of RDRAM at
0x80000000 and runs the game's NON_MATCHING C against it. Nothing in
`src.us.v11/` changes for this; the matching build is untouched.

```
make VERSION=us.v11 NON_MATCHING=1 -j8   # build_nm/ ELFs: the address source
make VERSION=us.v11 -j8                  # build/ ELFs: the reference for `check`
make -C port -j8                         # port/build/spike.exe
make -C port check                       # run it, compare with the original MIPS code
make -C port linkall                     # every hd_code game file in one exe
make -C port typemap ucode               # byte-order schema coverage / RSP microcode hashes
```

Needs `i686-w64-mingw32-gcc` (WSL), the project venv (pyelftools, unicorn),
host `gcc` (for `typemap`) and the ROM at run time (`ROM=`, default
`../baserom.us.v11.z64` or `~/blastcorps/baserom.us.v11.z64`). The exe gets
a Windows path through `wslpath -w`.

## How it fits together

1. **RDRAM.** `src/rdram.c` reserves and commits 8 MB at 0x80000000 with
   `VirtualAlloc`. The exe is linked `--large-address-aware` (without it,
   addresses at or above 2 GB don't exist for the process),
   `--disable-dynamicbase` and `--disable-reloc-section`. Absolute symbols
   must never be rebased, so the image always loads at 0x400000.
2. **Game C.** Each game file is compiled to assembly with
   `-DNON_MATCHING -O2 -fno-strict-aliasing -fwrapv -msse2 -mfpmath=sse`. The
   port's `include/port_ultratypes.h` is forced in. `-fno-builtin-{sin,cos}{,f}`
   keeps the libultra versions, because gcc otherwise turns sinf and cosf into
   `sincosf`.
3. **Pinning data (`tools/relabel.py`).** Every data object the C defines that
   the NON_MATCHING ELF places at an N64 address becomes an absolute symbol at
   that address. Its native initialiser is kept under `__native_<name>`, and
   `build/copytab.c` lists these. Start-up copies the initialisers over the
   image, already in host byte order and layout. The copy is checked against
   the N64 `st_size`. Function-local statics (`D_xxxxxxxx.N`) are pinned by
   name.
4. **Undefined symbols (`tools/gensyms.py link`).** Data symbols become
   `_name = 0x8xxxxxxx;` lines in `build/abs_syms.ld`, an implicit linker
   script. Functions that no linked object defines become traps in
   `build/stubs.c`. Compiler and C-runtime helpers come from libgcc/msvcrt.
5. **Image load (`rdram_load`).** The hd_code `.data`/`.rodata` gzip member
   (ROM 0x7D73B4) is inflated to 0x802E8BD0, and the front end (text+ucode,
   data) to 0x801E7000/0x80208040. Both `.bss` ranges are cleared, then the
   native initialisers are copied in. The hd_code text is not loaded; its
   tables are C (NM_PIN_HD_CODE).
6. **Byte order.** Image bytes that came from the ROM are big-endian. Code
   reads them natively only after they are swapped by type. `tools/typemap.py`
   derives a per-symbol leaf layout (offset, width) from the DWARF of every
   game file's declarations. `spike_main.c` swaps what its tests read. Stage 3
   applies the full map.

## The spike's tests (`src/spike_main.c`, reference `tools/n64ref.py`)

`n64ref.py` runs the same cases through the original code (`build/`, unicorn,
memory big-endian). `check` compares the outputs line by line:

| test | function | reads | typed | raw (no swap) |
|---|---|---|---|---|
| T1 | func_802AD7D4/7FC arcsine (69000.c) | u16 table D_802AD880 in a .text bin, defined in 69930.c | 65536/65536 | 9/65536 |
| T2 | func_802A56C4 (60D50.c) | s16 D_80305B90[][3] in a hd .data bin | 21/21 | 3/21 |
| T3 | func_802A5510 (60D50.c) | level asset (s32 offsets + s16 HeightZones) in the heap | 300/300 | 26/300 (274 access violations) |
| T4 | func_802CB690 (86ED0.c, vehicle helper) | .bss only; records its calls | 200/200 | 200/200 |

## Headless (stage 3): `make -C port headless`

`build/headless/bc_headless.exe ROM [options]` runs the whole game logic (every
hd_code and front-end game file, plus the ultralib gu, libm, printf and
libaudio objects compiled natively) on the platform layer in
`src/platform/`, with no graphics or audio output. Options are listed in
`src/platform/headless_main.c` (`--frames N`, `--dump F1,F2`, `--dump-every N`,
`--trace FILE`, `--input FILE`, `--gettime FILE`, `--frame-done FILE`,
`--eeprom FILE`, `--gfx-cycles N`, `--boot-count N`, `-v`...). A frame is one
frame-ending gfx task (the scheduler task with flag 0x40); dumps are the 8 MB
of RDRAM in host byte order, taken when that task starts.

The build differs from the spike's in a few ways:
- `-malign-double -mno-ms-bitfields`: u64/f64 members 8-aligned and GCC
  bitfield packing, so struct layouts match IDO's (MinGW's MS bitfields make
  the GBI's `Gfx` 16 bytes).
- `-DPORT_HOST` enables the game's port hooks (`include/game/port.h`):
  `PORT_SPIN()` in the two busy-waits on the VI counter (00000.c) and
  `PORT_FE_LOADED()` after the front end is inflated (46C20.c). Both expand
  to nothing in the N64 builds.
- hd_code files that are only SDK asm or RCP-register C (8FD90 ... A0A30) are
  left out; the ultralib os/io objects are replaced by the platform layer.
- Two ultralib libc files read a double's sign/exponent half-word at index 0
  (big-endian); sed makes copies with the little-endian index (`le/`).
- `port_on_dma` (the hook the load layer implements, `src/platform/port_dma.h`)
  is an identity stub while `src/load/` has no sources.
- `INTERIM=1` (build dir `build/headless-interim`) links stand-ins for the
  load layer (`src/platform/interim_swap.c`: typemap image swap, sequence
  headers, banks, the texture table, front-end scenes) so the platform can be
  exercised before the load layer lands. Not for use together with
  `src/load/`.

Platform model (`src/platform/`):

| piece | file | behaviour |
|---|---|---|
| threads, messages, events | os_thread.c | libultra's priority scheduler run cooperatively on host fibers: switches only where libultra would (block, ready a higher priority, interrupt); FIFO among equal priorities. The idle thread is parked when it drops to priority 0. |
| virtual clock | os_thread.c, os_time.c | 46.875 MHz counts; game code takes no time; the clock jumps to the next event (retrace every 781250 counts, timers, task completions) when every thread waits, or in `port_spin`. osGetTime/osGetCount read it; injection by file (`--gettime`), C hooks, `--boot-count`. |
| frame pacing | os_hw.c | a frame's gfx task completes `--gfx-cycles` after it starts (default one retrace), or at the retrace a `--frame-done` file gives (e.g. from an emulator trace). |
| VI | os_hw.c | the VI manager's retrace work: swap the framebuffer, send the osViSetEvent message every retraceCount retraces. |
| PI | os_hw.c | osPiStartDma = memcpy from the ROM + `port_on_dma`, completion message at once; osPiRawReadIo reads ROM words (the debug command line at 0xFFB000 is `--cmdline`, default empty). |
| SI | os_si.c, input.c | controller 1 from `--input` (or idle), osContInit's 0.5 s wait; EEPROM 4 Kbit in a file (raw bytes as the game writes them); no Controller Pak. |
| RSP/RDP | os_hw.c | gfx tasks dropped and counted (SP done, plus DP done for frame tasks); func_802A4B0C's cull test answered "visible"; audio tasks dropped, SP done. |
| AI | os_hw.c | a two-buffer DMA FIFO playing at the programmed rate in virtual time, so osAiGetLength (which sizes each audio frame) behaves; samples are discarded. The synthesizer runs (the game polls sequence/sound state). |
| front end | plat_core.c | data+bss snapshot at boot, restored after every reload. |
