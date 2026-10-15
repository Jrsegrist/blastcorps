# Blast Corps

[![CI](https://github.com/Jrsegrist/blastcorps/actions/workflows/ci.yml/badge.svg)](https://github.com/Jrsegrist/blastcorps/actions/workflows/ci.yml)

- This repo is about to contain a full decompilation of Blast Corps `(Japan)`, `(USA)`, `(USA) (Rev 1)` and `(Europe) (En,De)`.
- Naming and documentation of the source code and data structures are in progress.

It uses the following ROMs:

| no-intro                       | Location             | sha1                                       |
| ---                            | ---                  | ---                                        |
| `Blast Corps (USA)`            | `baserom.us.v10.z64` | `185a6ef7ba1adb243278062c81a7d4e119bda58c` |
| `Blast Corps (USA) (Rev 1)`    | `baserom.us.v11.z64` | `483f7161aea39de8b45c9fbc70a2c3883c4dea8c` |
| `Blastdozer (Japan)`           | `baserom.jp.z64`     | `b147fdbeb661c89107c440b00dc4810508f58636` |
| `Blast Corps (Europe) (En,De)` | `baserom.eu.z64`     | `460212600f8b9f0da95219c4c7330f2e626d9a7e` |

This repo does not include all assets necessary for compiling the ROMs.
A prior copy of the game is required to extract the assets.

# Clone the repo

Clone recursivley to initialize the splat submodule.

```
git clone https://github.com/retroplastic/blastcorps.git --recursive
```

If you cloned it without `--recursive`, you can initialize the submodule later.

```
git submodule init
git submodule update
```

# Build

This is a two-stage build; first stage is to extract the compressed section from the ROM, second stage is to extract/compile them.

Place a US Rev 1.0 ROM at the base of this repo.

## Set up Python for splat

```
virtualenv .env
. .env/bin/activate
pip install -r tools/splat/requirements.txt
```

## Stage 1

**Extract init, hd_code and hd_front_end code from ROM**
```
make VERSION=us.v11 extract
```
**Decompress hd_code and hd_front_end .text and .data sections**
```
make VERSION=us.v11 decompress
```
**Build ROM**
```
make VERSION=us.v11
```

## Stage 2 (Optional)

**Extract `init` + `hd_code` (TODO: `hd_front_end`):**
```
make VERSION=us.v11 -C blastcorps extract
```
**Compile ASM/C**
```
make VERSION=us.v11 -C blastcorps
```
**Compress compiled code and replace**
```
make VERSION=us.v11 -C blastcorps compress
```
**(re)Build ROM**
```
make VERSION=us.v11
```

## Decompiling a function

[`tools/mips_to_c`](tools/mips_to_c) (upstream: [matt-kempster/m2c](https://github.com/matt-kempster/m2c)) generates
a first-pass C guess from a disassembled function, using the project's real headers as context so types/signatures
for known libultra calls resolve correctly instead of showing up as `?`.

```
pip install graphviz
tools/m2c.sh blastcorps/asm/init/1A30.s
tools/m2c.sh -f func_80220E70 blastcorps/asm/init/1F40.s
```

The guess will not match byte-for-byte on its own — use `tools/asm-differ` to iterate against it until it does, then
move the function into `src.<version>/<segment>/<offset>.c` and flip the corresponding entry in
`<segment>.<version>.yaml` from `asm` to `c` (see `init.us.v11.yaml`'s `0x1660`/`0x3990` entries for examples,
including how to mark an irreducibly-asm function like a CP0 register accessor with `#pragma GLOBAL_ASM(...)`
instead of forcing it into C).

Note: many functions in `init` (and likely the low-level parts of `hd_code`) are standard N64/libultra boot and
hardware-init code shared across nearly every N64 game, not Blast Corps-specific logic — e.g. `0xA4600000`-range
addresses are PI (cartridge DMA) registers, and `0x1FC007FC` is the PIF RAM control register. It's worth checking
whether an existing N64 decomp (e.g. the [n64decomp](https://github.com/n64decomp) projects) already has a matching,
documented implementation before decompiling these from scratch.

## Decomp status

`python3 tools_port/decomp_status.py` (from `blastcorps/`, after a matching build) counts every
function of the three code segments by category. Permanent hand-written assembly is listed in
`tools_port/hand_asm.txt`; `-v` lists the functions that are not matched C.

| segment | functions | matched C | hand asm + NM C | hand asm, no C | near-miss | unmatched IDO C | IDO C matched | named |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| init | 43 | 31 | 0 | 12 | 0 | 0 | 100.0% | 43 (100%) |
| hd_code | 1400 | 692 | 681 | 27 | 0 | 0 | 100.0% | 1400 (100%) |
| hd_front_end | 155 | 147 | 6 | 0 | 2 | 0 | 98.7% | 155 (100%) |
| total | 1598 | 870 | 687 | 39 | 2 | 0 | 99.8% | 1598 (100%) |

(Earlier counts included 17 leftover SDK alias symbols, func_802D4020 = osInitialize and
the like, as matched hd_code functions: 1615 / 887.)  `--naming` adds the rename map's size,
the shared structs' named fields and the named constants (game modes, sounds, the vehicle
state offsets, vehicle types, surfaces, object kinds, level kinds, Yoshi windows, effect kinds).
After naming phase 4 (the data): 2075 data symbols named; 41 `D_XXXXXXXX` names are left in the
C and headers (4 statics, kept by rule, and 37 symbols with no evidence: unreferenced, only cleared,
or array elements that only comments mention); 226 of the shared structs' 258 fields are named (the
rest are padding or never read).

- **matched C**: compiled from C in the byte-matching build (game code and the ultralib-based SDK).
- **hand asm**: Rare's hand-written game code (hd_code 56040–8DDB0, the front end's 1B100
  wrappers), the libultra `.s` routines and the boot entry. They stay `GLOBAL_ASM` in the
  matching build; the port uses the `NON_MATCHING` C versions where they exist ("+ NM C"), and its
  platform layer replaces the SDK ones.
- **near-miss**: IDO-compiled functions still `GLOBAL_ASM` in the matching build, with
  `NON_MATCHING` C that the port uses (front end fe_pak_thread and fe_globe_draw_route).

## Naming conventions

Symbols start with splat's address names, `func_XXXXXXXX` (code) and `D_XXXXXXXX` (data), and keep them
until there is evidence for a real one: what the code does, read together with its callers and the data it
touches. A guess is not a name: when unsure, keep the address name and add a one-line comment saying what is
known.

| what | style | examples |
|---|---|---|
| a name the game or the SDK itself gives (assert and debug strings, file names, libultra, the SDK demos' scheduler and audio manager) | exactly that name | `levelno`, `playerNumber`, `yoshiState`, `__scMain`, `osSendMesg` |
| functions | `module_verb_object`, lower snake case; the module is the subsystem or the original file (`hd_` hd.c, `cam_` camera, `veh_` vehicles, `lvl_` levels, `obj_` objects and buildings, `mode_` game modes) | `hd_main_thread`, `cam_build_projection` |
| global variables | `g` + CamelCase | `gGameMode`, `gNextGameMode` |
| tables of ROM data, `.rodata` constants the `NON_MATCHING` C reads through `extern` | `g` + CamelCase, named for the content / the value's role | `gLevelTunes`, `gMathTwoPi` |
| ROM offsets of files and blobs (`D_00xxxxxx`) | `rom` + CamelCase of the file | `romLagpRaw`, `romMusicBankCtl` |
| segment addresses (`D_01xxxxxx`, `D_02xxxxxx`) | `seg1` / `seg2` + CamelCase | `seg2Base`, `seg1RspInitDl` |
| file-scope and function-local `static` data | keep `D_XXXXXXXX`: the port's `coffpin.py` pins statics at the address their name carries | |
| types | CamelCase | `DynamicBuf`, `LevelInfo` |
| struct fields | lowerCamelCase; unidentified ones stay `unkXX` / `padXX` (XX = the offset) | `modelId`, `unk5C` |
| constants and macros | UPPER_SNAKE_CASE | `MODE_LEVEL`, `TOPLEVEL_DL_SIZE` |

Each named function gets a one- or two-line comment on what it does, ending with its address, e.g.
`/* The main game thread: game mode switches, then the frame loop (0x80244930). */`.

**The rename map.** `blastcorps/tools_port/names.txt` holds every rename as `old = new` (fields as
`field Type.old = new`, renamed by hand: the compiler reports a use that was missed). It is also the alias
table for old names in notes and reports: `python3 tools_port/rename.py --which func_XXXXXXXX`.
`tools_port/rename.py` applies the map to the whole tree (C, headers, symbol_addrs and undefined_syms, the
check files, conventions.txt, hand_asm.txt, the port's tools and data files, READMEs), adds each new name to
its segment's `symbol_addrs` file at its address, and is safe to re-run. Tools that classify symbols by their
address-name prefix (`func_` = game function, `D_` = N64 data) ask `tools_port/names.py` for the old name
first, so a rename changes nothing they do. Names never change code: after a rename the matching ROMs,
the NON_MATCHING ROM and every check must come out the same.

**Shared structs and views.** The records several files read (PlayerInfo, LevelInfo, LevelObject,
VehShadow, AnimChannel, VehState) have one definition in `include/game/types.h` / `object.h`. A file
whose code reads the same memory with other offsets or types keeps a view type named for what it does
(e.g. 26570.c `PlayerHintView`, 01C40.c `LevelTimesView`), with the shared field names where they
overlap. Two lessons from naming phase 3: keep a view macro's form (`#define gX (*(T * N64P *) &gX)`
compiles differently in the NON_MATCHING build from a plain declaration of the same type, so the NM ROM
changes), and remove the C objects before a header-only check (the Makefile does not track header
dependencies: `rm build*/src.us.v11/*/*.c.o`).

**Applying the map on another branch** (before merging it with one that renamed):
```
cd blastcorps
python3 tools_port/rename.py                     # rewrite the branch's files
python3 tools_port/rename.py --check             # no old name left (also run by the CI)
make VERSION=us.v11 extract                      # the asm under the new names
make VERSION=us.v11 && make VERSION=us.v11 NON_MATCHING=1 nmrom
make -C port inputs                              # port/inputs: the new names
```
Then merge; a conflict is then a real one, not a rename. A file the branch adds is covered by the first step
(or `rename.py FILE...`).

## C tools

C tools from queueRAM's `blast_corps_docs`, `sm64tools` and other places.
They can be found in the `tools/src` subdirectory of this repo.

### Build the tools

```
meson build-tools
ninja -C build-tools
```

### Run the tools

```
./build-tools/tools/src/blast_textures
./build-tools/tools/src/gen_level_table
```

# Continuous integration

GitHub Actions (`.github/workflows/ci.yml`) runs on every push and pull request to `main`, without the ROM:

- **Windows**: builds the native Windows port (`blastcorps/port`, MSVC x64) from a clean checkout and packages it;
  the player package (`BlastCorps-port-<hash>.zip`) and its symbols (`-pdb.zip`) are attached to the run as
  artifacts. They contain no game data: the game reads it from the player's own ROM.
- **Linux**: the Python tools' self-tests and `make -C blastcorps/port norom-check` (the declared types against
  the committed `port/inputs`, the pointer-width audit, record layouts and the alignment check).

The matching ROM build, the NON_MATCHING build, `runchecks`, `check-inputs`, `verify` and `verify-levels` need the
ROM, so they stay local: see `blastcorps/port/README.md`, "Continuous integration".

# Related

* mkst's [blastcorps](https://github.com/mkst/blastcorps)

  Initial set up of splat and Makefile build this repo is based on.

* queueRAM's [blast_corps_docs](https://github.com/queueRAM/blast_corps_docs)

  The original repository this is based upon. The content can be found in `docs` and `tools`.

* queueRAM's [BlastCorpsEditor](https://github.com/queueRAM/BlastCorpsEditor)

  A C# level editor for blast corps.

* queueRAM's [sm64tools](https://github.com/queueRAM/sm64tools)

  A N64 rom manipulation tool silimar to splat, written in C.

* mkst's [gzip](https://github.com/mkst/gzip) branch

  Backport of the pre-1.5 bug behaviour of gzip to support the rare gzip format.

* ethteck's [splat](https://github.com/ethteck/splat)

  A binary splitting tool, used as subrepository in this project.

* [n64decomp](https://github.com/n64decomp)

  A collection of N64 decompilation projects.

* queueRAM's [Texture](https://github.com/queueRAM/Texture64)

  Can be used to view raw textures extracted from gzip. Works with mono.

