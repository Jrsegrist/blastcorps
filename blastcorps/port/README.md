# port/: the native Windows port

Blast Corps' NON_MATCHING C, compiled with MSVC for x64 Windows: `bc.exe`
(the game in a window, RT64 renderer), `bc_headless.exe` (the same game
logic, no output: the test vehicle) and the player package.  The N64's 8 MB
of RDRAM sits at 0x80000000 in the process and the game's data at its N64
addresses, in N64 layouts.  Nothing in `src.us.v11/` changes for this beyond
`NON_MATCHING`/`PORT_HOST` code; the matching build is untouched.

There is one build: MSVC (`CMakeLists.txt`), opened in Visual Studio or
driven from WSL.  Around it:

- **Build on Windows with Visual Studio**: what a player or developer needs;
  no WSL.  The facts the build takes from the decomp are committed in
  `port/inputs/`.
- **Verification from WSL** (`Makefile`, `msvc.mk`): `port/inputs`, the
  checks against the emulator and the original code, all run against the
  MSVC exes.
- **The clang check** (`port64.mk`): a clang x86_64 bc_headless of the same
  sources that nobody plays, for the checks MSVC can't do (pointer-width
  audit, record layouts, signed-overflow census) and as a second compiler
  whose per-frame traces must equal MSVC's.
- **The matching ROM build**: unchanged.

```
# Windows (x64 Native Tools Command Prompt, or Visual Studio: Open Folder > port/)
cmake --preset x64-release && cmake --build --preset x64-release   # bc.exe, bc_headless.exe
cmake --build --preset dist                                        # the player package

# WSL (needs the NON_MATCHING build: make VERSION=us.v11 NON_MATCHING=1 at the decomp root)
make -C port msvc                  # the MSVC exes, from WSL (MSVC_DIR on a Windows drive)
make -C port verify                # check-inputs, loadcheck, bc_headless vs the emulator
make -C port verify-levels         # every level vs the emulator
make -C port loadcheck dist compare strict-data typemap ucode
make -C port inputs check-inputs   # port/inputs: rewrite / check against the code
make -C port asan                  # bc_headless with AddressSanitizer
make -C port clang-check           # the clang check (ptrcheck64, headless64, traces vs MSVC)
make -C port overflow-census       # signed overflows at run time (clang -fsanitize)
```

## Build on Windows with Visual Studio

**Prerequisites**
- Visual Studio 2022 or later with the "Desktop development with C++"
  workload (MSVC x64 tools, a Windows SDK, "C++ CMake tools for Windows",
  which bring CMake and Ninja).  Tested with Visual Studio 2026 18.10
  (MSVC 14.51, CMake 4.3, Windows SDK 10.0.26100).
- Python 3 (python.org; only its standard library is used) on `PATH`, or
  `-DPython3_EXECUTABLE=...`: the build's generated sources come from the
  Python tools in `tools/` (pinning, tables, ultralib copies).
- Git for Windows: the repository and its ultralib submodule
  (`git submodule update --init lib/ultralib`), and the first configure
  fetches RT64.
- The ROM, Blast Corps (USA) (Rev 1), only to run the game (bc.exe asks for
  it with a file dialog, or takes it as its first argument).

Nothing else: the facts the build needs from the decomp's NON_MATCHING
build (data addresses, which symbols are functions, the N64 pointer slots,
the C declarations' types and the two link maps) are in `port/inputs/`,
which is part of the repository (made in WSL by `make -C port inputs`; see
"Verification from WSL").  It holds no ROM data.

**Building**
1. Visual Studio: File > Open > Folder..., the `blastcorps/port` folder.  It
   reads `CMakePresets.json`: choose "x64 Release" (the default, optimised,
   with a PDB), "x64 Debug" (no optimisation, for stepping through the game
   code), "x64 Release, bc_headless.exe only" (nothing downloaded) or
   "x64 Release + AddressSanitizer" (below).
2. The first configure downloads into `port/build/thirdparty` (shared by all
   presets): RT64 at the pinned commit with its submodules (git, ~500 MB) plus
   the port's patches 0003 and 0004 (`cmake/fetch_rt64.cmake`), the official
   DXC v1.9.2609 and SDL2 2.26.3 releases (SHA-256 checked).
3. Build > Build All: `out/build/<preset>/bc.exe` (with SDL2.dll,
   dxcompiler.dll, dxil.dll next to it) and `bc_headless.exe`.  The target
   `dist` makes the player package: `dist/BlastCorps-port-<hash>.zip` and
   `dist/BlastCorps-port-<hash>-pdb.zip` (bc.pdb, the map and the .syms
   that match the exe).
4. Run and debug: pick `bc.exe` as the startup item and press F5; arguments
   (the ROM, `--no-config` ...) go into its launch configuration
   (Debug > Debug and Launch Settings: `"args": [ "D:\\roms\\bc.z64" ]`).
   Breakpoints work in the game's C (`src.us.v11/hd_code/*.c`), the platform
   layer and RT64.  N64 memory sits at its N64 addresses, so a game variable
   is watched as `*(u64 *)0x80364A90` (the game mode).

From a command prompt ("x64 Native Tools Command Prompt for VS"):
`cmake --preset x64-release`, `cmake --build --preset x64-release`,
`cmake --build --preset dist`.  The Visual Studio generator works too
(`cmake -G "Visual Studio 18 2026" -A x64 ...`).  Keep the binary folder's
path short: RT64's object paths must stay under 250 characters.

**Heap checking: the x64-asan preset** (`BC_ASAN`; from WSL `make -C port
asan` -> `MSVC_DIR/Asan/bc_headless.exe`).  bc_headless with the port's host
code (load layer, platform, audio, crash reporter: everything that uses the
C heap) built `/fsanitize=address`: a heap overrun or underrun, a use after
free, a double or bad free stops with AddressSanitizer's report (the access,
the block, where it was allocated and freed, with source lines);
`--crash-test heapover` checks that it does.  It runs as bc_headless does
(same options, same per-frame trace), about five times slower.  The game C
is not instrumented: it has no heap, and ASan's redzones around globals
would cover the N64 data that follows them.  Two things make it work with
the fixed-address RDRAM:
- ASan's run time puts its shadow memory in the first large free range of
  the address space while its DLL initialises, before any code of the exe
  runs; that range starts at 0x7FFF0000 and covers 0x80000000.  So N64
  memory comes from `bc_rdram.dll` (`src/rdram_asan_dll.c`), a DLL with no
  code and one 8 MB section, linked at a fixed base of 0x7FFF0000 with 64 KB
  section alignment: its section starts at 0x80000000, the loader maps it
  with the exe's imports before any DLL initialises, and ASan's shadow goes
  elsewhere.  `rdram_map` makes that range writable and clears it instead of
  VirtualAlloc'ing it.
- ASan's heap lives above 4 GB.  Only host code keeps heap blocks (the game
  is never given one), so the ASan build lets `rdram_map`'s below-4-GB check
  pass for the heap; a heap address that did reach N64 memory would be cut
  to 4 bytes and fault at once.
The DLLs (`bc_rdram.dll`, `clang_rt.asan_dynamic-x86_64.dll` from the
compiler's folder) are copied next to the exe.  This replaces the retired
32-bit build's page-heap wrapper (`checked_heap.c`, mingw `--wrap`).

## How the port works

### Memory model

N64 memory is at 0x80000000-0x80800000 in the 64-bit process (`src/rdram.c`
reserves and commits it with `VirtualAlloc`), so every N64 address is a
valid host pointer and N64-layout data keeps 4-byte pointers: RAM stays
byte-identical to the N64's apart from byte order.  Everything the game can
be given a host address of must be below 4 GB: the exe (linked at a fixed
base 0x40000000, `/FIXED /DYNAMICBASE:NO /HIGHENTROPYVA:NO`, no relocations;
code reaches RDRAM RIP-relative, < 1.1 GB away), the heap and the fiber
stacks (bottom-up allocations without high-entropy VA; `rdram_map` and the
fiber trampoline check it).

The game and SDK headers and the game C mark what is an N64 pointer
(`port/include/port_n64ptr.h`; the marks expand to nothing in the IDO builds,
`include/2.0I/PR/ultratypes.h`):

| mark | meaning (MSVC and clang) | where |
|---|---|---|
| `T * N64P x` | `__ptr32 __uptr`: a 4-byte pointer, zero-extended when loaded | record fields, pinned globals, casts that read an N64 pointer (`*(u8 * N64P *) (p + 8)`), functions taking pointers to such pointers |
| `N64FN(T) x` | a u32 (clang can't use `__ptr32` function pointers): `N64FN_SET`, `N64FN_GET(T, x)` | libaudio's handlers in N64 memory |
| `N64_IPTR(x)` | `(unsigned) x`: zero-extend | a signed 32-bit address cast to a pointer |
| `N64_A32` | `unsigned` | an int holding an address added to a pointer |
| `N64_DPTR(x)` | 0, the value comes from the pointer table | pointers in static initialisers of N64 data |
| `N64_KEEP` | `used` | a pinned static clang would fold away |

ultralib's ul_* objects see ultralib's own headers: `tools/ulhdr64.py`
copies them with the marks listed in `data/ultralib_n64ptr.txt`, and
`tools/ulsrc64.py` rewrites the libaudio sources' uses of N64FN fields, the
bank loader's s32 address offsets (bnkf.c) and `sizeof(ALFilter *)` (the bus
source tables live in the audio heap, which the game sizes exactly: 0 bytes
left over).  `tools/lesrc.py` makes little-endian copies of five SDK
sources (`le/`): two libc files read a double's sign/exponent half-word at
index 0, gu's sinf/cosf initialise their double constants as big-endian word
pairs, and the Controller Pak's inode/ID byte pairs and a directory entry's
never-written bytes (see the tool).

### Pinning data, tables, the link

- **Pinning (`tools/coffpin.py`).** Every data object the C defines that the
  NON_MATCHING ELF places at an N64 address is taken out of its COFF object
  (built with one section per variable, `/Gw`): an initialised one keeps its
  bytes as `__native_<name>` (the copy table), a bss one becomes undefined,
  and every relocation to it goes to an undefined `<name>`.  Function-local
  statics (`D_xxxxxxxx`) are pinned by name.
- **Absolute symbols (`gensyms.py linkcoff`, `tools/rdramobj.py`).** The N64
  data symbols the objects use become absolute COFF symbols in one object;
  functions nothing defines become traps (`stubs.c`).  link.exe resolves a
  REL32 fixup against an absolute symbol as image base + value and an ADDR64
  one as the value, and refuses ADDR32NB ones (MSVC indexes global arrays
  off `__ImageBase`): the symbols are written as address - image base, and
  coffpin adds the base to ADDR64/ADDR32 addends and resolves ADDR32NB
  itself.  It also handles `/Gw`'s per-variable COMDAT sections, MSVC's names
  for local statics and its COMMON symbols (C tentative definitions).
  (RDRAM as a section of the image at 0x80000000 would avoid absolute
  symbols, but the image would then cover 0x7FFE0000, where Windows maps
  KUSER_SHARED_DATA: such an exe doesn't start.)
- **Copy table (`gensyms.py copytab64`).** Start-up copies the native
  initialisers over the loaded image, already in host byte order and layout;
  each copy is checked against the N64 `st_size`.
- **Pointer table (`gensyms.py ptrtab-in`, `inputs/nmptrs.txt`).** Every
  pointer slot of the pinned C objects (the NON_MATCHING objects' R_MIPS_32
  data relocations) with the word the NM ELF holds there, written after the
  copy: the `N64_DPTR` initialisers (strings, tables of N64 addresses) get
  the N64's own values.
- **Image load (`rdram_load`).** The hd_code `.data`/`.rodata` gzip member
  (ROM 0x7D73B4) is inflated to 0x802E8BD0, and the front end (text+ucode,
  data) to 0x801E7000/0x80208040.  Both `.bss` ranges are cleared, the load
  layer swaps the image bytes by type (below), then the native initialisers
  and the pointer table go in.  The hd_code text is not loaded; its tables
  are C (NM_PIN_HD_CODE).

### MSVC specifics

- Game code: `/O2 /Ob1` (only `inline` functions are inlined; `/Ob0` in the
  files that read the clock), `/GH` (an empty `_pexit` call before each return
  keeps tail calls as calls: return addresses name their callers, which
  `--clock`/`--sync` key on and crash reports use).  `--sync`'s
  function-entry points and `--calls` patch only the functions they name, at
  start-up, into their entry (`plat_host.c host_entry_hook`, `entry_x64.asm`,
  the linker's `/FUNCTIONPADMIN`): the hook runs before any instruction of
  the function.  (MSVC's own `/Gh` hook comes after the prologue, and the
  compiler schedules body code, N64 memory accesses among it, before it.)
- MSVC has no `-fwrapv`: the signed adds and multiplies that really overflow
  are wrapped explicitly (`PORT_WRAP_MUL`/`PORT_WRAP_ADD`, `game/port.h`:
  the two random-number generators in 23C20.c and the debris size in
  77E20.c), found by the clang check's overflow census.  No
  `-fno-strict-aliasing` equivalent is needed (MSVC does no type-based alias
  analysis), nor `-malign-double` (8-byte members are naturally aligned).
  The GBI's bit-fields are written for Microsoft's layout rules where they
  differ (`gbi.h`, `rdb.h`; `layout_check.c` asserts every GBI and ABI
  record's size and offsets and checks the bit positions at start-up).
  `sinf`/`cosf` stay the SDK's (`#pragma function`).  MSVC doesn't assume
  16-byte alignment of N64 data (`tools/align64.py` over its objects finds
  only stack saves), so the clang build's IR alignment pass has no MSVC
  counterpart.
- Crash reports name functions and source lines from the exe's PDB
  (dbghelp, with a time limit); the minidump opens in Visual Studio with the
  PDB.  The static C runtime (`/MT`): the package needs no Visual C++
  redistributable.
- `-DPORT_HOST` enables the game's port hooks (`include/game/port.h`):
  `PORT_SPIN()` in the two busy-waits on the VI counter (00000.c),
  `PORT_FE_LOADED()` after the front end is inflated (46C20.c) and the rest
  below.  Both expand to nothing in the N64 builds.  hd_code files that are
  only SDK asm or RCP-register C (8FD90 ... A0A30) are left out; the ultralib
  os/io objects are replaced by the platform layer.

### Byte order on load (`src/load/`)

Memory is host-endian (little-endian): everything the game's C reads
natively is swapped once, by the width the code reads it at, when it arrives
from the ROM. Textures, palettes, pictures, compressed streams and the byte
streams the code reads bytewise (5CB60.c's BE16U/BE16S/BE32) stay big-endian.

- **Data images** (hd_code .data/.rodata at 0x802E8BD0, front end at
  0x80208040): `rdram_load` calls `port_load_image_hd/fe` (swap.c) with the
  table `swaptab.c` that `tools/swaptab.py` generates from
  1. `tools/typemap.py`: the DWARF of every game file's declarations (host
     gcc in WSL; `inputs/typemap.txt`) and its per-file
     `#define D_x ((T *) D_x)` struct views (merged: the most detailed view
     first; arrays run past splat's mid-array symbols);
  2. `data/image_widths.txt`: the widths the NON_MATCHING code reads each
     address at (traced, below); the trace wins where it reads one wider field
     over narrower declared ones;
  3. `data/image_static.txt`: fixed-address accesses of the native code, for
     bytes 1-2 leave untyped (made once by `tools/mixscan.py --widths` over
     the retired i686 build's gcc assembly);
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
- **Graphics data the renderer reads** (`src/load/gfx_fix.c`): the
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
  game's own loaders and consumers (`bc_loadcheck.exe`,
  `src/loadcheck_main.c`), native vs the original code in unicorn
  (`tools/loadref.py`): T5 the D_802E8BF8 alias, T6 520 texture decodes, T7
  every level's object stream, T8 every level's height zones; with a trace,
  T9 checks every traced read of every traced asset right after loading.

### bc_headless and the platform layer

`bc_headless.exe ROM [options]` runs the whole game logic (every hd_code and
front-end game file, plus the ultralib gu, libm, printf and libaudio objects
compiled natively) on the platform layer in `src/platform/`, with no
graphics or audio output. Options are listed in
`src/platform/headless_main.c` (`--frames N`, `--dump F1,F2`, `--dump-every N`,
`--trace FILE`, `--input FILE`, `--gettime FILE`, `--frame-done FILE`,
`--eeprom FILE`, `--mpk FILE`, `--print`, `--cmdline STR`, `--gfx-cycles N`,
`--boot-count N`, `--syms FILE`, `-v`...). A frame is one frame-ending gfx
task (the scheduler task with flag 0x40); dumps are the 8 MB of RDRAM in host
byte order, taken when the main thread sends that task to the scheduler.
The build writes `bc_headless.syms` (`tools/mapsyms.py`: link.exe's map as
`nm -n` output) for `--syms`.

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
records, `PORT_WRAP_*` for signed overflow. Tools: `tools/eepsave.py FILE
[OUT --set OFF:W=VAL]` decodes or edits an .eep (fixing its CRC).

### bc.exe: the game in a window

`bc.exe ROM [bc_headless options] [--api d3d12|vulkan]
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

**RT64** (github.com/rt64/rt64, MIT) at the pinned commit 43373749, with
`rt64/patches/0003` (`RT64_NATIVE_HOST_LAYOUT`) and `0004` (the L3D
handler), is fetched by `cmake/fetch_rt64.cmake` and built with MSVC as a
static subproject.  To change RT64, edit the fetched tree
(`port/build/thirdparty/rt64`, each patch a local commit there), rebuild,
and regenerate the patch with `git diff`.

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
identical per-frame traces (retrace, time, mode, level, frames in mode, game
retrace counter). Their RDRAM differs only in what the renderer writes
(colour and depth images: framebuffers, Z, the shadow and other
render-to-texture images), the staged ucode, the graphics data gfx_fix.c
converted, and pointers to the exes' own code/data.

### Player setup and package

bc.exe is a Windows GUI program (`/SUBSYSTEM:WINDOWS`): double-clicking it
opens no console. Its log goes to stderr when the parent gave it one (WSL, a
redirect, a script: everything below works unchanged), otherwise to
`bc.log` next to the exe (or `%TEMP%`). With `host_gui` set, `host_fatal`,
the crash reporter, C++ `terminate` and bad options also show a message box.

**`--no-msgbox`** (both exes; every script that runs one passes it:
compare.py's `native`, so `verify`, `verify-levels`, `level`, `run`): no
dialog of any kind, for automated runs. Errors and crash reports go only to
the log (stderr) and the crash files; `host_message` just logs; bc.exe shows
no ROM file dialog (no ROM -> exit 2); Windows' fault, critical-error and
WER dialogs are off (`SetErrorMode`, `WerSetFlags`); the C runtime's assert
and abort messages go to stderr; `MessageBox*` calls from the exe's own
imports (RT64) and SDL2.dll only log their text; SDL assertions become crash
reports. It is read by `host_crash_init` before anything else (so even a bad
option or a missing ROM stays quiet) and accepted by the shared option
parser (`host_main`, headless_main.c). `BC_NO_MSGBOX=1` in the environment
does the same, but WSL doesn't pass the environment to Windows exes.
Exit codes: 0 done, 1 usage (bc_headless), 2 fatal error (`host_fatal`;
bad option, missing ROM ...), 4 crash.

**Crash reports** (`src/platform/crash.c`, both exes). `host_crash_init`,
first in `main`, installs an unhandled-exception filter (re-armed after RT64
starts), `std::terminate` (bc.exe), `SIGABRT` and SDL assertion handlers,
and starts a reporter thread that waits. A crash on any thread or fiber hands
the exception to that thread (its own stack: stack overflows work; no C
runtime locks or heap), which writes:
- to stderr (bc.exe: its log) and `bc-crash-YYYYMMDD-HHMMSS.txt`: the
  exception (code, fault address as module + RVA, read/write address),
  registers, the host thread (main = the game's fibers; the fiber), the game
  state (game thread, frame, retrace, mode, next mode, level, frames in mode,
  game VI counter), and a backtrace: return addresses found on the stack,
  call-site checked and unwound with the x64 unwind tables, named with
  source lines from the exe's PDB (dbghelp, with a time limit) or the
  nearest DLL export; a frame whose direct call targets the function below
  it is confirmed, others are marked `?` (indirect calls, tail calls or
  stale);
- `bc-crash-YYYYMMDD-HHMMSS.dmp` (`MiniDumpWriteDump` from System32's
  dbghelp.dll, loaded at start-up): threads, indirectly referenced memory, the
  8 MB RDRAM and the exe's .data/.bss (~17 MB); with `--dump-dir`, also
  `frame_9999999.bin` as before. The files are written by a second waiting
  thread with a 20-second limit: MiniDumpWriteDump (and creating a file)
  allocates from the process heap, whose lock a crash inside the heap leaves
  held for good (it happened: see "Rare boot crash" below); the report on
  stderr is complete by then, and an unfinished dump deletes itself. It also
  suspends every other thread of the dump, so the reporter is left out of
  the dump (`IncludeThreadCallback`) and keeps its time limit; the dumper
  first runs `HeapValidate` and writes no dump when that finds the heap
  corrupt; the PDB lookup first checks that the heap's lock is free.
They go to `--crash-dir DIR` (both exes; compare.py passes the native run's
folder), else bc.exe: bc.log's folder (the exe's), bc_headless: the current
folder, falling back to `%TEMP%`. Then bc.exe shows a message box (unless
`--no-msgbox`) and the process ends with exit code 4 through
`TerminateProcess` (no DLL detach or atexit code runs in a crashed process;
`host_fatal` ends the same way, code 2). `--crash-test KIND[:FRAME]` crashes
on purpose (av, div, stack, thread, abort, fatal, box, heaplock, heapbad,
heapover; bc.exe also cxx).
Example (bc_headless --crash-test av:5):

    CRASH: access violation (exception 0xC0000005) at 0x00004000A350 = bc_headless.exe+0xA350
      read of address 0x000000000010
      rax=0000000000000010 rbx=0000000040939410 rcx=0000000040105E10 rdx=0000000000002002
      ...
      rip=000000004000A350 eflags=00010246
      host thread 24892 (main: the game's fibers), fiber 0000000002AC4A00; Blast Corps port dev, exe bc_headless
      game: thread 3@80310BD0, frame 5, retrace 49, mode 0x0000000000000010 (next 0x0000000000000000), level 0, frames in mode 4, game VI counter 49
    backtrace (return addresses found on the stack; ? = not confirmed by the call chain, may be stale):
      #0   00004000A350  bc_headless.exe+00A350  host_crash_test_frame+0x100 (crash.c:1120)
      #1   0000400AECB8  bc_headless.exe+0AECB8  plat_on_frame+0x128 (plat_core.c:112)
      #2   0000400A9376  bc_headless.exe+0A9376  osSendMesg+0x46 (os_thread.c:295)
      #3   00004004B40F  bc_headless.exe+04B40F  func_80284E54+0x17F (405F0.c:97)
      #4   00004008ECE9  bc_headless.exe+08ECE9  func_801EF4AC+0x79 (07800.c:380)
      #5   000040024270  bc_headless.exe+024270  func_80244930+0x1930 (00000.c:977)
      #6 ? 0000400A8DEE  bc_headless.exe+0A8DEE  fiber_main+0xE (os_thread.c:138)
      ...
    minidump: ...\bc-crash-20261007-212029.dmp

**Rare boot crash** (heap corruption seen 3 times in ~250 runs before
784dbcf: `free()` of the sound bank map after the swap, once in `sscanf`, at
retrace 30-107). Cause: `swap_bank` (load/port_assets.c) walks the first
copy of each sound bank (at 0x8004B400, where it is decompressed) and meets
offsets 0xFFFFFFFB, 0xFFFFFFFC and 0xFFFFFFFF; the old `bank_once` check
`off + size > len` wrapped and passed, so `done[off] = 1` wrote 1-5 bytes
*before* the map, into the heap block header, whenever the byte there read 0
(an encoded header byte: rarely, and differently every run). Fixed by
784dbcf (bounds without wrap-around), found and confirmed with the (since
retired) 32-bit build's page-heap wrapper: the old walk stopped at every
boot ("block of 19209 bytes was written before its start (-4)"), the
current code ran 2000 boots, 60 bc.exe level runs and all 60 levels under
it without a fault.  The x64-asan preset is the tool for this now.

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
- **The package** (CMake target `dist`, `dist/make_dist.py`; from WSL
  `make -C port dist` -> `port/build/dist/`): `BlastCorps-port-<hash>/` and
  its zip: bc.exe, SDL2.dll, dxcompiler.dll, dxil.dll, `README.txt` (from
  `dist/README.txt`: running, controls, bc.ini, saves, known issues) and
  `THIRD_PARTY_LICENSES.txt` (every licence from the trees the build used:
  RT64 and its contrib code, DXC, SDL2; it fails if one is missing), plus
  `BlastCorps-port-<hash>-pdb.zip` (bc.pdb, map, syms).  No ROM data.  The
  exe's icon is drawn by `tools/make_icon.py`; `res/bc.rc` adds the version
  resource (git hash) and a manifest.

### Audio (`src/audio/`)

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

## Verification from WSL

The decomp side (WSL Ubuntu: IDO, splat, the NON_MATCHING build) makes the
MSVC build's inputs and runs every check against the MSVC exes.  Needs the
project venv (pyelftools, unicorn), host `gcc` (typemap.py), Visual Studio
on the Windows side, the NON_MATCHING ELFs (`make VERSION=us.v11
NON_MATCHING=1` at the decomp root), the matching ELFs (`build/`, for
loadcheck's reference) and the ROM (`ROM=`, default `../baserom.us.v11.z64`
or `~/blastcorps/baserom.us.v11.z64`).  The exes get Windows paths through
`wslpath -w`.

**`port/inputs/`** (`make -C port inputs`, `gensyms.py inputs`): what the
MSVC build takes from the NON_MATCHING build, as text files, so the Windows
side needs neither the ELFs nor pyelftools:
- `addrs.txt`: every data symbol at an N64 address, with its size;
- `elfsyms.txt`: every ELF symbol's name, address, kind (func/data) and size;
- `nmptrs.txt`: every pointer slot (R_MIPS_32 data relocation) of the NM *C*
  objects' data with the word the NM ELF holds there (a symbol's address
  plus the C source's addend; data the build extracts from the ROM is never
  read);
- `typemap.txt`: the declared scalar leaves of every data-image symbol
  (`tools/typemap.py`: host gcc's DWARF of the game C);
- `hd_code.us.v11.map`, `hd_front_end.us.v11.map`: the NM link maps
  (section addresses and sizes per object, symbols).
Facts of the decomp's own code and its NM link only: no ROM bytes (asset
contents, images, values read from the ROM).  The folder is committed.
After a change to the game code (anything that moves NM data, changes a
declaration or a pointer initialiser), run `make -C port inputs` and commit
the result; **`make -C port check-inputs`** (part of `verify`) brings the
NON_MATCHING build up to date, makes the inputs again in
`build/inputs-check` and fails if they differ from `port/inputs`.

**`make -C port msvc`** builds the MSVC exes from WSL (`tools/msvc_build.sh`:
the sources, `port/inputs` included, mirrored to `MSVC_DIR` on the Windows
side, default `%LOCALAPPDATA%\blastcorps-msvc`, because MSVC compiling over
`\\wsl.localhost` is slow; Visual Studio found with vswhere; CMake + Ninja
in its x64 environment; `MSVC_CONFIG` Release (default), Debug or Asan;
`MSVC_PYTHON`, `MSVC_THIRDPARTY` if needed) -> `MSVC_DIR/<config>/`.  Every
target below builds what it needs this way.  (The old `-msvc` names,
`verify-msvc` etc., still work.)

Other generated files live in `port/build/gen/`: `addrs.txt`,
`typemap.txt`, `typemap_all.txt` (every symbol, for strict mode),
`swaptab.c` and `le/` (for the clang check), `facts.txt`.

### Comparing with the emulator: `make -C port compare DEMO=n`

`tools/compare.py` runs the attract mode in mupen64plus (tools_port/m64trace,
spec `tools/cmp_spec.py`), feeds bc_headless the emulator's clock, and
compares RDRAM frame by frame.  The exe is `CMP_EXE` (the make targets pass
the MSVC build's; without it, the Release build in `MSVC_DIR`); its native
runs go to `...-nativemsvc` folders (`CMP_TAG`).  It uses the NON_MATCHING
test ROM (`build_nm/blastcorps.nm.us.v11.z64`, `make ... nmrom`), which runs
the same C as the exe, so a difference is a port problem (byte order,
platform, host compiler), not a rewrite bug; `CMP_KIND=base` uses the
original ROM. The emulator run (log + RDRAM dumps every 10 frames, ~1 GB per
demo) is cached in `CMP_CACHE` (default `~/cmp_cache/attract-nm-demoN`) and
redone when cmp_spec.py or the ROM changes. Output: the timeline (submission
retrace, game mode, level, frames in mode, game retrace counter per frame),
then per compared frame "match" or the differing runs with symbols and the
asset an address was loaded from.

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
  by cmp_spec.py; patched in at start-up, see "MSVC specifics") is a switch
  point: the native thread spins (interrupts and higher-priority threads
  run) until the emulator's time for it, and until the game's retrace
  counter D_803156C4 reads what it read there. Not for the scheduler and
  audio threads (interrupt driven; their call order follows the RSP).
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
the exe its argument registers' home area, which may be stale). Never-written
record padding is logged by the exe (`port_garbage`, "load: garbage") and
skipped; the game's reads of such stale bytes get the N64's big-endian view
(`port_n64_byte`, from the unit every swap recorded; `port_vtx_stale` does
the same for the HUD quads 42240.c builds without writing their colour
bytes, which the RSP reads).

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

### Quick regression check: `make -C port verify`

Rebuilds the NM test ROM, runs `check-inputs`, builds the MSVC bc_headless
and bc_loadcheck, runs `loadcheck`, then `tools/compare.py verify --frames
VERIFY_FRAMES` (default 1500: boot, logos, front end and attract demo 0): the
emulator side (NM ROM, `--lle`, dumps every 10 frames) is cached in
`$(CMP_CACHE)/verify-nm-N` and redone only when the ROM (by content: `make
nmrom` rewrites it every time), `cmp_spec.py` or the tracer change (the whole
check takes about 5 minutes then, 1.5 minutes with the cache).  Reported:
frames whose timeline matches, RAM dumps that match (lenient, and strict with
the layouts of `make -C port strict-data`), the visibility tests (emulator
LLE vs the exe's model), the emulator's audio tasks vs the interpreter, the
main thread's switch points used.  It fails when a number is worse than
in `data/verify_expect.txt` (`key MIN`, `key =N`, `key <=N`).
`VERIFY_FRAMES=14498 make -C port verify` covers all nine demos (about 25
minutes the first time, ~12 GB of dumps).

### Every level: `make -C port verify-levels`

The attract demos load 9 of the game's 60 levels (`D_802E8BDC` 0-59; the
type byte of each `D_802E8F94` record is in `data/levels.txt`).  To reach any
level with the same input on both sides, `tools/compare.py level --level L`
plays `data/levels_input.txt` (made by `tools/levels_input.py`): START / A
from controller read 400 (title, new game, player A, the opening sequence),
A on the globe at read 985, A taps for the intro pages, then from read 1220 a
driving pattern (accelerate, turn both ways, brake and reverse, stop, Z out,
walk, A, Z in, ...).  At frame 930, before the globe first opens, both sides
write L to player 0's "last level" byte (`D_80364AF0[0].pad0[8]`, 0x80364AF8:
00000.c reads it once, on the first entry to the globe, 0x4000), so the globe
opens centred on L and A selects it; the game loads it its own way (mission
intro, bonus-level page, sequence).  The poke is `CMP_POKE` / `compare.py emu
--poke` in the emulator and `--poke F:ADDR:SIZE:VALUE` in the exes (applied
when frame F is sent, after its dump); the input is `CMP_INPUT` / `--input`.
In the emulator the pad is set at the entry of each `osContStartReadData`,
so read n gets line n exactly (the input plugin's own count includes 2 more
PIF commands by the first frame).  `--shots F,..` also saves the emulator's
window (Glide64mk2) at those frames.

`make -C port verify-levels [LEVELS=a,b-c] [LEVEL_JOBS=2]` runs every level
for 1900 frames (the level from frame ~1046-1172, ~730-855 frames in it),
emulator side cached in `$(CMP_CACHE)/level-nm-LL` (about 3.5 minutes per
level the first time; 14 dumps: every 20 frames from frame 1000 to 1100,
where the level loads and the comparator's stale-trailer rule learns the old
level's heap, then every 100), and compares
the timeline and the RAM dumps (lenient: in a level, heap data lands on
declared arrays such as the frame buffers, so the strict typing reports
noise; `compare.py level --strict` has it).  Expectations per level are in
`data/verify_levels_expect.txt` (`L key VALUE`, `* key VALUE` for all).
All 60 levels take about 85 minutes the first time (two emulators at a
time; ~5.6 GB of cache) and 11 minutes cached.

Results (Oct 2026): every level is reached, no crash, the timeline matches
on all 1900 frames in 59 levels (49, the ending: two mission-intro frames
are submitted a retrace early natively), and all 14 RAM dumps match in 55;
the rest differ in what the comparison can't follow: 47 a streaming
decompressor's progress between frames, 58 the music's sequence position
(audio thread timing), 23 and 45 RDP-rendered words on the level-end screen.
bc.exe, following the emulator's clock, shows the same pictures as the
emulator's Glide64mk2 window (radar colour aside, a Glide64 error) and gives
bc_headless's per-frame trace in all 60.  The fixes this found (levels agent):
a crash in level 13 (an untyped trigger table), the trail zones' table, the
area test's halves, a front-end static in host .bss, two mid-function
retrace reads (PORT_GVI), and the stale-byte emulation: display lists,
matrices, vertices and text quads built in heap memory record their units,
and the never-written vertex fields of several builders (sprites, boxes,
globe icons, water) get the N64's old bytes.

Debugging what native code wrote: `--watch F:ADDR[:N]` logs the first N
writes to ADDR's 4 KB page from frame F on (rip and the stack's code
pointers; WSL doesn't hand environment variables to the exe, hence an
option); the emulator side is `tools_port/m64trace` with `WRITES`/`ONWRITE`
over `cmp_spec.py` (scratchpad tools of the levels work).  `m64widths.py`
takes `W_INPUT` / `W_POKE` / `W_SAVEDIR` to trace a level's access widths.

## The clang check (`port64.mk`)

`make -C port clang-check`: the same game, ultralib and platform sources
built by clang 18 as an x86_64 `build/headless64/bc_headless.exe`
(`--target=x86_64-w64-mingw32 -fms-extensions`, mingw-w64's x86_64 binutils
and CRT, all from apt), checked, then run against the MSVC bc_headless.
Nobody plays it and nothing ships from it; it exists for what MSVC can't
check and as a second compiler for the same sources:

1. **`ptrcheck64`**: `tools/ptrcheck64.py` over clang's AST of every game,
   platform and ultralib file: host-width pointer fields and pinned globals,
   casts to host-pointer pointers, signed int -> pointer casts (they
   sign-extend: `N64_IPTR`), sizeof of host pointers, pointer + (s32)
   pointer; `tools/layout64.py`: every record laid out i686 vs x86_64 (sizes
   and offsets must be equal); `data/ptrcheck64_allow.txt` lists the
   host-only records.  clang also errors on a pointer that changes address
   space under another pointer (`u8 **p = D_x` with D_x an N64P array).
2. **`headless64`** with **`tools/align64.py`**: the x86-64 ABI gives global
   arrays of 16 bytes or more 16-byte alignment, and clang assumes it for
   every array, extern ones too: it used `movaps` on an 8-aligned N64 array.
   The game code is compiled to LLVM IR, `tools/llalign64.py` lowers every
   alignment above 4 on memory accesses and N64 globals, then the code is
   generated; align64.py checks the objects (no 16-byte-aligned access
   except stack and constants).  `-fno-inline-functions` and
   `-fno-optimize-sibling-calls` for the game, as MSVC's `/Ob1 /GH`;
   `-finstrument-functions` gives `--sync` its entry points.  Pinning is the
   same `coffpin.py`; the absolute symbols come from `gensyms.py link64`
   (nm over the objects; GNU ld and lld link them alike), the pointer table
   from the NM ELFs (`gensyms.py ptrtab`) rather than `inputs/nmptrs.txt`.
3. **Traces** (`tools/tracecmp.py`): the clang exe and the MSVC exe side by
   side over the attract demos (4000 frames) and every level (1900 frames,
   `data/levels_input.txt` and the globe poke, as verify-levels), on the
   free-running virtual clock (no emulator); every per-frame trace must be
   identical and every run must exit 0.  `CLANG_CHECK_ARGS` narrows it
   (`--levels 0-9 --attract 2000`); `LEVEL_JOBS` runs at a time.

**`make -C port overflow-census`**: MSVC has no `-fwrapv` and no
signed-overflow sanitizer, so the census is a clang build
(`build/hl64ubsan`, `-fno-wrapv -fsanitize=signed-integer-overflow` with a
minimal runtime of our own, `tools/ubsan_minimal.c`) run over the attract
demos and every level (`tools/overflow_census.py`); it fails if any signed
add or multiply overflows that the source doesn't wrap explicitly
(`PORT_WRAP_*`).  `OC_ARGS`: e.g. `--levels 0-9`.  (Its build crashes in
level 47: 01C40.c func_801E8EB8 reads `bios[slot]` with slot 4, past its
4-entry array, in the player select's biography mode; a latent
out-of-bounds read of the original code that the normal builds survive.)

## The matching ROM build

Unchanged: `make VERSION=us.v11 -j` at the decomp root builds the matching
ROM and checks it (the three OK lines), `make VERSION=us.v11 NON_MATCHING=1`
the NON_MATCHING ELFs and `nmrom` the NM test ROM; `bash
tools_port/runchecks.sh` the rewrite checks.  Port changes to game code are
`NON_MATCHING`/`PORT_HOST` only (`include/game/port.h`, the N64P marks), so
the matching build stays byte-identical.
