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
make -C port headless                    # build/headless/bc_headless.exe: the game, no output
make -C port game                        # build/game/bc.exe: the game in a window (RT64)
make -C port verify                      # quick regression check against the emulator
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
- **Graphics data the renderer reads** (`src/load/gfx_fix.c`, stage 4): the
  schemas swap an asset's display-list area as u32 words (the CPU walks and
  patches commands as words), but such an area can also hold vertices,
  viewports, lights and texels, which then end up in the emulator's layout
  (two halfword pairs and a byte-reversed colour word per Vtx).  Likewise the
  parts of the data images no declaration or traced read typed stay ROM
  bytes, and some are display lists, vertices and matrices only the RSP reads
  (e.g. the front-end list at 0x80208FA8 that level 0's lists call).  The
  schemas and `port_load_image_*` register both kinds of area; before bc.exe
  draws a task, `port_gfx_fix_task` follows its display lists with the real
  segment table and converts what they use there to the native layout, each
  word once per load and only while it still holds what was loaded (data the
  game wrote since is left alone).  bc_headless doesn't call it.  Vertices
  in the data images that other typing rules made words get the `vtx`
  override (`data/image_overrides.txt`; the CPU never reads them).
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
`--eeprom FILE`, `--mpk FILE`, `--print`, `--cmdline STR`, `--gfx-cycles N`,
`--boot-count N`, `-v`...). A frame is one
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
| SI | os_si.c, input.c | controller 1 from `--input` (or idle), osContInit's 0.5 s wait; EEPROM 4 Kbit in a file (`--eeprom`, mupen64plus's 512-byte .eep format). |
| Controller Pak | pif.c | `--mpk FILE` (mupen64plus .mpk: four 32 KB paks, controller 1's first; created formatted if missing): the front end's own SDK Pfs objects run natively over a PIF emulation (status, pak read/write); the system blocks (ID, inode, directory) are converted by layout at the pak boundary. |
| saves | save.c | the save thread's routines (0E7B0.c, `PORT_SAVE_*` hooks) work on a big-endian copy of each record, so CRCs and files are the N64's: EEPROM files are byte-identical to mupen64plus's for the same play (checked: new game, level completion with best time), and load either way. |
| RSP/RDP | os_hw.c | gfx tasks dropped and counted (SP done, plus DP done for frame tasks; bc.exe draws them with RT64, same completion times); func_802A4B0C's cull test answered by a CPU model of its ucode (below); audio tasks run at once on the microcode interpreter (src/audio/, "Audio" below), SP done. |
| AI | os_hw.c | a two-buffer DMA FIFO playing at the programmed rate in virtual time, so osAiGetLength (which sizes each audio frame) behaves; each accepted buffer goes to the output (`--wav FILE`; bc.exe's speakers), which never feeds back into the timing. |
| front end | plat_core.c | data+bss snapshot at boot, restored after every reload. |
| debug output | headless_main.c | `--print`: the game's debug printf (func_8029A7E4, empty on the N64) to stderr with the frame number; `--cmdline "-c"` turns on the game's debug cheats (C-right + Z completes the level). |

Host-order rules the game C follows under `PORT_HOST` (include/game/port.h):
`PORT_HALF(i)` for u16 views of 32-bit words (the N64 Mtx's elements; the
port keeps Mtx words in host order), `PORT_SHAMT(n)` for variable shift
amounts that can reach 32 (MIPS uses the low 5 bits), `PORT_SAVE_*` for save
records. Tools: `tools/eepsave.py FILE [OUT --set OFF:W=VAL]` decodes or edits
an .eep (fixing its CRC).

## Windowed (stage 4): `make -C port game`

`build/game/bc.exe ROM [bc_headless options] [--api d3d12|vulkan]
[--shot F1,F2..] [--shot-dir DIR] [--shot-every N] [--no-pace] [--scale N]
[--mute] [--volume N]`
plays the game in a window: bc_headless's game and platform objects plus
`src/live/live_rt64.cpp` (SDL2 window and input, the RT64 renderer). The
exe needs `dxcompiler.dll`, `dxil.dll` and `SDL2.dll` next to it (the build
copies them). Options are listed at the top of live_rt64.cpp; `--frames N`
and the other bc_headless options work as there.

Keys: arrows = stick, X = A, C = B, Z or Space = Z, Enter = START, A/S =
L/R, I J K L = C buttons, T F G H = D-pad, M = sound on/off, - and = =
volume, Esc quits; an SDL game controller
(XInput pads etc.) maps the same way (left stick, A, B/X, triggers = Z,
Start, shoulders, right stick = C buttons, D-pad). `--input FILE` replays a
recording instead.

**Building RT64.** The first `make -C port game` runs `rt64/build_rt64.sh`:
it clones RT64 (github.com/rt64/rt64, MIT) at the pinned commit 43373749
into `$TP/rt64_bc/src` (`TP` default `~/thirdparty`; outside the repo),
applies `rt64/patches/0001-0004`, fetches DirectX-Headers and DXC v1.9.2609
(the Linux dxc compiles the shaders, the Windows x86 DLLs run next to the
exe: both must be the same version), and builds a static i686 library with
mingw-w64 (`JOBS`, default 2). It reruns only when a patch changes. The
patches: 0001/0002 the mingw/i686 build (stage-0 spike); 0003
`RT64_NATIVE_HOST_LAYOUT`; 0004 the L3D handler. To change RT64, edit
`$TP/rt64_bc/src` (each patch is a local commit there), rebuild with
`ninja -C $TP/rt64_bc/build-i686 rt64`, and regenerate the patch with
`git diff`.

**RT64_NATIVE_HOST_LAYOUT** (`src/common/rt64_rdram_layout.h` in the
patched tree): RT64 normally reads RDRAM in the emulator's layout (each
32-bit word a host u32 of the big-endian word: the byte at A is at A^3).
The port's RDRAM is the game's own memory: structures in host order with
their fields where the C declares them, byte streams (textures, palettes,
colour and depth images, ucode) as big-endian bytes. The switch reorders
RT64's Vtx/Light structs, the viewport indices and the G_MW_LIGHTCOL word,
makes the TMEM loads read bytes without the XOR, drops the word swaps when
images go between RDRAM and the GPU (the shaders already produce and take
big-endian pixels), and hashes the ucode from a word-swapped copy (the
database holds emulator-layout hashes). Gfx commands and Mtx are host u32
words in both layouts.

**L3D.** The front end's ucode (D_80207090, slot 0) is Fast3D's line build;
RT64 knew it by hash ("L3D Blast Corps") but had no handler. Patch 0004
adds `GBIUCode::L3D`: F3D plus G_LINE3D (0xB5, `gSPLineW3D`), drawn by
`RSP::drawLine` as a screen-space quad 1.5 + wd/2 pixels wide (the vertices
are moved perpendicular to the line in clip space and mapped back through
the inverse MVP). The globe's route lines use it.

How the platform drives it (HostLive in `plat_host.h`; NULL in
bc_headless):
- boot: the ucode text and data RT64 hashes (D_802E53F0, D_8030E390,
  D_80210690) are put back at their addresses as ROM bytes (hd_code's text
  isn't loaded natively; nothing native reads them).
- `osSpTaskStartGo`: a gfx task goes to RT64 (`loadUCodeGBI` +
  `processDisplayLists`, after `port_gfx_fix_task`). func_802A4B0C's
  visibility test (ucode D_802E77B0, whose RDP output the CPU reads) stays
  with the platform: os_hw.c `cull_visible` evaluates its list (projection
  and modelview, 8 box corners, 12 triangles): visible iff a triangle is not
  entirely outside one of x = +-w, y = +-w, z = -w in clip space and
  clipping it to those planes leaves a polygon. This model matched the real
  ucode, run on an LLE RSP (mupen64plus + cxd4 + angrylion-rdp-plus), in all
  3921 tests of the attract demos; `BC_CULL_LOG=1` logs every test. (With
  `--clock` the emulator's word is used instead, and the model is checked
  against it: the stats line `cull: N tests, H not visible; ... the model
  disagrees with D`. Comparison runs take the word from an LLE RSP,
  `compare.py emu --lle`; mupen64plus's HLE RSP never runs this ucode, so
  its word would always say "visible".)
  RT64 raises the SP/DP interrupts while it processes the task;
  they are counted, and the game hears about completion from the same
  virtual-time model as bc_headless (`--gfx-cycles`, `--frame-done`).
- every retrace: the VI registers the VI manager would program for the
  frame on screen (os_hw.c, from the game's OSViMode, osViBlack and
  osViSetSpecialFeatures, which edits the control word cumulatively like
  the ROM's: the game turns VI gamma off, so RT64 applies none; bc.exe logs
  every VI status change), `updateScreen`, SDL events, and pacing: virtual
  retrace N is shown at start + N/60 s (`--no-pace` runs flat out; more than
  a quarter second behind restarts the reference).
- input: controller 1 from the keyboard/pad when no `--input` file is given.

Debugging what is drawn: `--dl-dump F[:N]` / `--dl-dump-every N` print
display lists (with each G_VTX's first vertex in both layouts);
`--dl-skip LO:HI` turns the triangles whose commands lie in that physical
range into no-ops (bisect which list draws a shape); `--gfx-fix-log` with
`BC_GFX_WATCH=addr` reports what gfx_fix.c does to one word. Data-image
vertices typed as words by some declaration need a `vtx` line in
`data/image_overrides.txt` (four so far, all K0-addressed G_VTX targets in
the hd data image now match the ROM layout).

Same game logic: with the same input, bc.exe and bc_headless produce
identical per-frame traces (2001 frames compared: retrace, time, mode,
level, frames in mode, game retrace counter). Their RDRAM differs only in
what the renderer writes (colour and depth images: framebuffers, Z, the
shadow and other render-to-texture images), the staged ucode, the graphics
data gfx_fix.c converted, and pointers to the exes' own code/data.

## Player setup and package (stage 6)

bc.exe is a Windows GUI program (`-mwindows`): double-clicking it opens no
console. Its log goes to stderr when the parent gave it one (WSL, a
redirect, a script: everything above works unchanged), otherwise to
`bc.log` next to the exe (or `%TEMP%`). With `host_gui` set, `host_fatal`,
the crash reporter, C++ `terminate` and bad options also show a message box
(`BC_NO_MSGBOX=1`, passed with `WSLENV`, only logs them: automated tests).

`src/live/live_setup.cpp`:
- **ROM.** A ROM argument is used as given; otherwise `rom =` from bc.ini;
  otherwise (or if that one fails the check) a Windows open-file dialog
  (GetOpenFileNameW), and the choice is written back to bc.ini.  Every ROM is
  read as .z64, .v64 (byte pairs swapped) or .n64 (little-endian words;
  `rom_normalise` in rdram.c, which `rdram_load` also uses) and must have the
  SHA-1 of Blast Corps (USA) (Rev 1); US v1.0, Japan, Europe, other games and
  modified dumps get their own message.  `--no-rom-check` skips the check
  (the NON_MATCHING test ROM).
- **bc.ini** next to the exe (`--config FILE` for another), created on the
  first start from the commented defaults (the text in live_setup.cpp is
  the defaults): `[game]` rom, saves, controller_pak; `[video]` api, scale,
  fullscreen, vsync; `[audio]` volume, mute; `[keyboard]`/`[controller]` one
  line per N64 button (SDL key names / SDL game-controller names, axes as
  `rightx+`), the analog stick's source and dead zone, quit.  Lines it
  doesn't understand are listed in one warning box; their defaults stay.
  Command-line options override it.
- **Saves**: `saves/blastcorps.eep` (and with `controller_pak = 1`
  `saves/blastcorps.mpk`) next to the exe; `--saves DIR`, `--no-saves`,
  `--no-pak`, `--eeprom`/`--mpk` override.  With a pak the game keeps its
  progress on the pak and never touches the EEPROM (checked: level completion
  with a pak = 0 EEPROM accesses), so the default is no pak.
- **`--no-config`**: no bc.ini, the built-in defaults, no save files unless
  `--eeprom`/`--mpk`/`--saves`: bc.exe then runs exactly like bc_headless
  with the same options.  Tests and comparisons should pass it (with a ROM
  argument), so a developer's bc.ini can't change what they measure.
- Keys: F1 puts the bindings in the window title, Alt+Enter / F11 switch
  fullscreen (desktop resolution; RT64 keeps 4:3), the window is resizable.
- Pacing: each retrace's picture is handed to RT64 when its virtual time is
  due (60 per second of real time, QueryPerformanceCounter, 1 ms timer
  resolution), *before* the game works on the next retrace, so the hand-overs
  are regular; vsync (`vsync = 1`, plume's swap chain) then shows it at the
  next refresh.  `--pace-log FILE` writes the hand-over and present times
  and logs a summary at exit.  Measured on this PC (60 Hz display, AMD
  integrated GPU, 1500 retraces of attract mode): 60.000 retraces/s; hand-over
  interval sd 0.6-2 ms (was 7-8 ms when the wait came after the hand-over),
  97-99.7 % within 2 ms of 16.67 ms; the misses are the first second of demo
  0 (level start: up to 50-77 ms behind with D3D12 while shaders compile,
  ~14 ms with Vulkan), then a catch-up.  Same per-frame trace with vsync
  on/off, windowed/fullscreen, D3D12/Vulkan.
- `make -C port dist`: `build/dist/BlastCorps-port-<hash>/` and its zip:
  bc.exe (debug info stripped), the three DLLs, `README.txt` (from
  `dist/README.txt`: running, controls, bc.ini, saves, known issues) and
  `THIRD_PARTY_LICENSES.txt` (`dist/third_party.sh` gathers every licence
  from the RT64 tree, DirectX-Headers, the DXC release and the mingw-w64/GCC
  runtime copyright files; it fails if one is missing).  No ROM data.  The
  exe's icon is drawn by `tools/make_icon.py`; `res/bc.rc` adds the version
  resource (git hash) and a manifest.

## Audio (stage 5): `src/audio/`

The game's audio thread (22EE0.c, Rare's audiomgr) builds a command list
each audio frame with libaudio (the old-SDK synthesizer, compiled natively
from the ul_* objects) and starts an M_AUDTASK with the ROM's audio
microcode (D_802E68F0, the "Blast Corps / Diddy Kong Racing" variant of
aspMain).  `src/audio/aspmain.c` executes that command list as the RSP
would: a model of DMEM (the microcode's map: segment table 0x320,
parameters 0x360, command buffer 0x380, ADPCM table 0x4C0, sample buffers
from 0x5C0, state scratch 0xF90), SP DMA with its 8-byte alignment, and
each command's vector-unit arithmetic (accumulator width, saturation,
rounding, vector flags) as the microcode does it.  The constant tables
(resampler filter, masks, ramps) come from the task's ucode data at run
time, read from the ROM's data image; nothing ROM-derived is in the source.
`port_audio.c` connects it to the platform: osSpTaskStartGo runs the task,
osAiSetNextBuffer hands the finished buffer to the output.

RDRAM layout at the interpreter's boundary (the bus in port_audio.c): the
command list is host-order u32 words (like display lists); A_LOADADPCM's
codebooks / pole-filter coefficients and A_SETLOOP's loop states are
host-order s16 (the load layer swaps them in the sound banks; libaudio
computes the coefficients and copies the loop state natively); everything
else the microcode moves stays in N64 byte order: ROM sample data, the
microcode's own state blocks (ADPCM, resampler, envelope mixer, pole
filter), reverb delay lines and the output buffers, which no C code reads.
The AI plays big-endian L/R pairs; the output converts them.

Outputs: `bc_headless --wav FILE` (the stream the game hands the AI, plus
`FILE.frames`: game frame -> sample offset; `--wav-all` also keeps buffers
a full AI FIFO drops, i.e. every task's output), deterministic; bc.exe queues
the same buffers to SDL audio at the AI rate (osViClock / dacrate, 22047 Hz)
with a little resampling (at most 0.5%) holding the device queue near
60 ms against drift between the paced virtual clock and the sound card; a
queue past 0.3 s (after a stall) is dropped.  `--mute`, `--volume N`, keys
M (mute), - and = (volume).  `--no-audio` skips the audio tasks (the game
logic is the same either way: bc.exe and bc_headless give identical traces
and identical WAVs).

Verification (`tools/audio/`, Linux tools; the LLE RSP is cxd4,
CC0-licensed, from `~/thirdparty/ref`, loaded at run time as a test oracle,
not shipped or copied):
- `rsp_tap.so`, an RSP plugin for mupen64plus: every audio task runs on the
  LLE RSP (the ROM's real microcode) and on aspmain from the same RDRAM and
  DMEM copy; all 8 MB of RDRAM are compared afterwards.  Other tasks go to
  `TAP_HLE` (rsp-hle).  `TAP_CAPTURE=file` saves each task's inputs and
  results.  `ai_dump.so`, an audio plugin, writes the AI stream to a WAV.
- `bc_headless --audio-capture FILE[:N]` records the native tasks;
  `asp_lle FILE build/hd_code.us.v11.bin` hosts the LLE plugin itself and
  runs them with the ROM's rspboot and microcode (from the build's
  decompressed segment); `asp_replay` reruns captures on aspmain.
- `wavcmp cmp REF.wav TEST.wav [SECTIONS]` aligns two recordings window by
  window and reports correlation, SNR and bit-exact samples.
- `compare.py emu` takes `CMP_TRACER` (a tracer with these plugins), so one
  emulator run gives both the clock logs for bc_headless and the sound.
  `asp_dump` prints captured command lists (`-sizes`: samples per task),
  `asp_listdiff A B` compares two captures' lists command by command.
  `BC_AILEN_SEQ=1` (WSLENV) makes `--clock`'s osAiGetLength values go to
  the audio thread in call order, for audio comparisons.

Results (Oct 2026): every audio task of the emulator's attract cycle
(17,236 and 17,243 tasks in two runs, base and NM ROM) leaves all 8 MB of
RDRAM and the sample-buffer DMEM identical to the LLE microcode's; so do
3,958 of bc_headless's own tasks under asp_lle.  With the emulator's clock
and its audio frame sizes, bc_headless's command lists equal the ROM's
(addresses aside) until the first sound the game starts in a different
audio frame (task 328, ~11 s), and the WAVs are bit-identical up to there;
the rest of the 132 s compared differs only by that event timing (band
spectrum within 0.2 dB, loudness within 0.1 dB; see the agent notes).  The
interpreter skips empty command lists (the microcode would run one
command out of a 256-row DMA of junk).  Not exercised by this game's lists
(so checked only against the microcode's code, not run): RESAMPLE flag 2
(the restored 16-byte input tail), ENVMIXER without A_AUX, commands 16+.

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
two u16 where the code reads a u32 still "matches" (see strict mode below).
`tools/compare_ignore.txt` lists what legitimately differs, with
reasons: framebuffers, Z-buffer, RSP buffers, per-frame display lists, code,
thread stacks and OSThreads, libultra internals the platform keeps
elsewhere, scheduler timing/history, and the audio subsystem (its section
of the file; `diff --audio` compares it, see below).

Strict mode (`diff --strict`, after `make -C port strict-data
LC_TRACE=trace.txt`): each byte also gets the width its readers use, from
the declarations of every symbol (`TYPEMAP_ALL=1 typemap.py`), the traced
reads of the data images and of every traced asset (placed where the exe's
load log put it), and `data/image_overrides.txt`; a word that matches only
under some other layout is reported. C objects' .data/.rodata, textures
and a few buffers whose declared type isn't their readers' (`Layout.UNTYPED`)
stay lenient. More emulator inputs: `Q` lines (the game retrace counter
where `PORT_GVI` marks a mid-frame read, `cmp_spec.py GVI_FUNCS`), and
`calls --args` compares the first argument words (`emu --calls` logs a0-a3,
the exe the first four stack words; build with `HL_EXTRA=-fno-inline` for
functions gcc inlines). Never-written record padding is logged by the exe
(`port_garbage`, "load: garbage") and skipped; the game's reads of such
stale bytes get the N64's big-endian view (`port_n64_byte`, from the unit
every swap recorded; `port_vtx_stale` does the same for the HUD quads
42240.c builds without writing their colour bytes, which the RSP reads).

**LLE RSP (`emu --lle`, the default for `run` and `verify`).** The
emulator runs the RSP tasks whose results the game reads on cxd4 (an LLE
RSP, CC0, from `~/thirdparty/ref`, loaded at run time as an oracle) through
`tools/audio/rsp_tap.so`: every audio task (each also checked against
the port's interpreter) and func_802A4B0C's visibility test
(`TAP_LLE_UCODE`); graphics stay on the HLE plugin; `ai_dump.so` writes the
emulator's sound to `EMUDIR/emu.wav`.  The emulator log also has
osAiGetLength's real values (logged in the caller after the call returns:
at the callee's `jr ra` the debugger saw a stale v0, which once made the
native audio frames absurd, the audio thread stop after ~3,990 tasks and
the timeline slip at frame ~3505) and each audio frame's sample count (Z
lines).  The sound player's event-posting calls (1A630.c sndPlaySfx and
friends) are entry switch points, so the game thread posts each sound
before or after the audio thread's frame as in the emulator.  An RDP-done
handler that ran before its own retrace's handler in the emulator (the
DP interrupt's message overtook the VI manager's) gets its completion just
before that retrace natively (`inject` counts them).

Results (Oct 2026, all nine attract demos, 14,498 frames, LLE run): the
timeline (retrace, mode, level, frames in mode, game VI) is identical on
every frame; RAM dumps (every 10th frame) 1445 of 1449 match, and strict
mode finds no word that matches only leniently in any of them (the 4 left:
the scheduler's cmdQ.validCount at the dump instant, when the audio
thread's timer fires a little earlier natively than the game thread's
frame send); 4,177 visibility tests, the native model agrees with the LLE
RSP's answer on all of them; 16,986 emulator audio tasks identical to the
interpreter's.  `diff --audio` (the audio heap, synthesizer, sequence and
sound players, audio manager): the audio state matches through frame 2370
except the padding and unused union bytes of ALEvent copies in the event
queues (stack garbage on both machines); from frame 2380 (demo 1) some
voice/DMA state differs.  Sound (`--wav` vs the emulator's LLE WAV, 564 s,
1 s windows): waveform correlation median 1.000 in every section, the
title and logos bit-exact, demo 0 98% of samples bit-exact, the rest
44-88% bit-exact with a median SNR of ~75 dB.

## Quick regression check: `make -C port verify`

Rebuilds the NM test ROM and bc_headless, runs `loadcheck`, then
`tools/compare.py verify --frames VERIFY_FRAMES` (default 1500: boot,
logos, front end and attract demo 0): the emulator side (NM ROM, `--lle`,
dumps every 10 frames) is cached in `$(CMP_CACHE)/verify-nm-N` and redone
only when the ROM (by content: `make nmrom` rewrites it every time),
`cmp_spec.py` or the tracer change (the whole check takes about 5
minutes then, 1.5 minutes with the cache).  Reported: frames
whose timeline matches, RAM dumps that match (lenient, and strict with the
layouts of `make -C port strict-data`), the visibility tests (emulator LLE
vs the exe's model), the emulator's audio tasks vs the interpreter, the
main thread's switch points used.  It fails when a number is worse than
in `data/verify_expect.txt` (`key MIN`, `key =N`, `key <=N`).
`VERIFY_FRAMES=14498 make -C port verify` covers all nine demos (about 25
minutes the first time, ~12 GB of dumps).
