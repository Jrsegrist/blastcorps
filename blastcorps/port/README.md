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
6. **Byte order.** Image bytes that came from the ROM are big-endian; the load
   layer (`src/load/`, next section) swaps them by type before the native
   initialisers go in.

## The spike's tests (`src/spike_main.c`, reference `tools/n64ref.py`)

`n64ref.py` runs the same cases through the original code (`build/`, unicorn,
memory big-endian). `check` compares the outputs line by line:

| test | function | reads | typed | raw (no swap) |
|---|---|---|---|---|
| T1 | func_802AD7D4/7FC arcsine (69000.c) | u16 table D_802AD880 in a .text bin, defined in 69930.c | 65536/65536 | 9/65536 |
| T2 | func_802A56C4 (60D50.c) | s16 D_80305B90[][3] in a hd .data bin | 21/21 | 3/21 |
| T3 | func_802A5510 (60D50.c) | level asset (s32 offsets + s16 HeightZones) in the heap | 300/300 | 26/300 (274 access violations) |
| T4 | func_802CB690 (86ED0.c, vehicle helper) | .bss only; records its calls | 200/200 | 200/200 |

## Byte order on load (`src/load/`, stage 3)

Memory is host-endian (little-endian): everything the game's C reads
natively is swapped once, by the width the code reads it at, when it arrives
from the ROM. Textures, palettes, pictures, compressed streams and the byte
streams the code reads bytewise (5CB60.c's BE16U/BE16S/BE32) stay big-endian.

- **Data images** (hd_code .data/.rodata at 0x802E8BD0, front end at
  0x80208040): `rdram_load` calls `port_load_image_hd/fe` (swap.c) with the
  table `build/swaptab.c` that `tools/swaptab.py` generates from
  1. `tools/typemap.py`: the DWARF of every game file's declarations and its
     per-file `#define D_x ((T *) D_x)` struct views (merged: the most
     detailed view first; arrays run past splat's mid-array symbols);
  2. `data/image_widths.txt`: the widths the NON_MATCHING code reads each
     address at (traced, below); the trace wins where it reads one wider field
     over narrower declared ones;
  3. `data/image_static.txt`: fixed-address accesses of the native code
     (`tools/mixscan.py --widths`), for bytes 1-2 leave untyped;
  4. `data/image_overrides.txt`: hand decisions (D_802F46C0 DL commands as
     words, the D_802E8BF8 flag/word alias kept big-endian, struct copies).
  Bytes from C objects need nothing: their native initialisers are copied
  over the image. `make -C port typemap` prints the coverage.
- **Assets**: `port_on_dma` (every PI copy, from the platform) and
  `port_on_load` (after the game decompressed a block: hooks in 46C20.c
  func_8028B4C4 and 5CB60.c's packed loader) classify the ROM range
  (port_assets.c) and apply the schema (schemas.h): the texture table, sound
  banks (walked like alBnkfNew), the sequence file and tunes, levels (header
  offsets, triangles, collision cells, lifts, group chain, display lists,
  segment-8 vertices), packed objects (buildings), vehicle models
  (parts, animations, vertices, display lists), the demo recordings, the
  front-end scenes and the static segment. Texture entries are swapped
  around 60F60.c's decoder (`port_texture_input/output`, NON_MATCHING hooks):
  s16 tokens in, big-endian texels out.
- **Exceptions fixed in NON_MATCHING C** (`PORT_HOST`): 00000.c
  `port_802E8BF8_word` (the u8 EEPROM flag is word 0's MSB of a word table),
  196F0.c (the front-end background pictures are tinted on big-endian pixels).

Tools:
- `tools/m64widths.py ROM OUT VIS` (MODE=nm for the NON_MATCHING test ROM,
  the default; MODE=base for the original): mupen64plus access-width tracer.
  Memory breakpoints (physical ranges; the core reports the aligned word, the
  byte address is recomputed from the instruction) over the data images and
  every loaded asset; per (region, offset, width, R/W, function). Run from the
  project root; about 25 minutes for the 30000 VIs of the nine attract demos.
  `EXTRA=name:lo:hi,...` adds regions (.bss state blocks).
- `tools/widths.py image|facts|assets`: reduce traces.
- `make -C port loadcheck [LC_TRACE=trace.txt]`: real assets through the
  game's own loaders and consumers, native vs the original code in unicorn
  (`tools/loadref.py`): T5 the D_802E8BF8 alias, T6 520 texture decodes, T7
  every level's object stream, T8 every level's height zones; with a trace,
  T9 checks every traced read of every traced asset right after loading.

## Headless (stage 3): `make -C port headless`

`build/headless/bc_headless.exe ROM [options]` runs the whole game logic (every
hd_code and front-end game file, plus the ultralib gu, libm, printf and
libaudio objects compiled natively) on the platform layer in
`src/platform/`, with no graphics or audio output. Options are listed in
`src/platform/headless_main.c` (`--frames N`, `--dump F1,F2`, `--dump-every N`,
`--trace FILE`, `--input FILE`, `--gettime FILE`, `--frame-done FILE`,
`--eeprom FILE`, `--gfx-cycles N`, `--boot-count N`, `-v`...). A frame is one
frame-ending gfx task (the scheduler task with flag 0x40); dumps are the 8 MB
of RDRAM in host byte order, taken when the main thread sends that task to
the scheduler.

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
  (big-endian), and gu's sinf/cosf initialise their double constants as
  big-endian word pairs (`du` in guint.h); sed makes little-endian copies
  (`le/`).
- `port_on_dma` (the hook the load layer implements, `src/platform/port_dma.h`)
  comes from `src/load/` (an identity stub while that has no sources), and
  the load layer's generated `swaptab.c` is linked too.
- `-fno-optimize-sibling-calls` everywhere and `-fno-inline` in the files
  that call osGetTime/osGetCount: a return address names the real caller,
  which `--clock`/`--sync` key on (below). The link also writes
  `bc_headless.syms` (`nm -n` of the exe) for `--syms`.

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

## Comparing with the emulator: `make -C port compare DEMO=n`

`tools/compare.py` runs the attract mode in mupen64plus (tools_port/m64trace,
spec `tools/cmp_spec.py`), feeds bc_headless the emulator's clock, and
compares RDRAM frame by frame. It uses the NON_MATCHING test ROM
(`build_nm/blastcorps.nm.us.v11.z64`, `make ... nmrom`), which runs the same C
as the exe, so a difference is a port problem (byte order, platform, host
compiler), not a rewrite bug; `CMP_KIND=base` uses the original ROM. The
emulator run (log + RDRAM dumps every 10 frames, ~1 GB per demo) is cached in
`CMP_CACHE` (default `~/cmp_cache/attract-nm-demoN`) and redone when
cmp_spec.py or the ROM changes. Output: the timeline (submission retrace,
game mode, level, frames in mode, game retrace counter per frame), then per
compared frame "match" or the differing runs with symbols and the asset an
address was loaded from.

A frame is the main thread handing the scheduler its frame task (osSendMesg
to D_80315440 with flag 0x40): the same program point on both machines; the
exe dumps there (`--dump`, `--dump-every`). Subcommands (`compare.py -h`):
`emu`, `inject`, `native`, `diff`, `frames`, `hex` (one region side by side),
`calls` (with `emu --calls F,G` and the exe's `--calls F,G`: the calls of
chosen functions per frame, to find where execution forks), `demos`, `run`.

What the emulator log gives the exe:
- `--clock`: every osGetTime / osGetCount / osAiGetLength result, keyed by the
  calling function and the retrace it was read in, and the visibility test's
  answer (func_802A4B0C's RDP output word) keyed by frame; other osGetTime
  calls get the emulator's clock (mupen64plus counts 811092 per VI; the
  mapping is in the file). Timers run in emulator counts.
- `--sync`: the CPU time model. Game code takes no time natively; every
  message call (osSendMesg/RecvMesg/JamMesg/StartThread), main-thread clock
  read, and entry of a function that reads the game's retrace counter (found
  by cmp_spec.py; the game files are built `-finstrument-functions`) is a
  switch point: the native thread spins (interrupts and higher-priority
  threads run) until the emulator's time for it, and until the game's
  retrace counter D_803156C4 reads what it read there. Not for the scheduler
  and audio threads (interrupt driven; their call order follows the RSP).
- `--frame-done` / `--frame-sp`: when each frame task's RDP and RSP parts
  finished; `--boot-count`: the power-on time when hd_code starts.

Word comparison: each 32-bit word matches if some layout agrees (u32, two
u16, u16 + two bytes, four bytes, an u64 pair, a u32 at 2 mod 4), if both are
pointers to the same function / the exe's own constant data, or an empty
thread queue (__osThreadTail vs NULL). This is lenient: a field swapped as
two u16 where the code reads a u32 still "matches" (a strict, typed mode is
future work). `tools/compare_ignore.txt` lists what legitimately differs, with
reasons: framebuffers, Z-buffer, RSP buffers, per-frame display lists, code,
thread stacks and OSThreads, libultra internals the platform keeps
elsewhere, scheduler timing/history, and the audio subsystem (the native
synthesizer is ultralib's newer libaudio, and the RSP task order isn't
modelled, so audio frames and the audio heap layout differ).
