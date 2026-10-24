# bcfiles: the level and model file library

`blastcorps/tools_port/bcfiles/` is a Python package (standard library only)
that reads every level, vehicle model, front-end scene and building from the
player's own ROM into Python objects, writes them back byte for byte, and
puts edited files back into a ROM. It is the file layer for a level editor;
the formats are described in [files](systems/files.md) and
[levels](systems/levels.md). Nothing ROM-derived is in the repository: the
tools read the ROM you give them and write to the folder you name.

## Command line

Run from `blastcorps/` (or anywhere, with the path to the package):

```bash
python3 tools_port/bcfiles list ROM                  # every file: kind, ROM range, name, sizes
python3 tools_port/bcfiles dump ROM level:2          # a level's header and records as text
python3 tools_port/bcfiles dump ROM model:driver --all   # a model, with its display lists decoded
python3 tools_port/bcfiles dump ROM object:12 --json # a building as JSON
python3 tools_port/bcfiles extract ROM OUTDIR        # every file decompressed, plus JSON
python3 tools_port/bcfiles build ROM OUTDIR NEW.z64  # the extract's (edited) JSON back into a ROM
python3 tools_port/bcfiles gltf ROM level:1 OUTDIR   # glTF 2.0 + PNG textures, for Blender
python3 tools_port/bcfiles roundtrip ROM             # parse + write + JSON every file, compare
python3 tools_port/bcfiles mod-check ROM MOD.bcmod    # a level mod: hashes and changes against the ROM
python3 tools_port/bcfiles mod-apply ROM MOD.bcmod OUTDIR    # the edited level files (level_NN.bin)
python3 tools_port/bcfiles mod-export ROM MOD.bcmod NEW.z64  # a ROM with the mod's levels in it
```

`python3 -m bcfiles ...` works too with `tools_port` on `PYTHONPATH`. FILE is
`level:N` or `level:NAME`, `model:NAME`, `scene:NAME`, `object:N` (an
object-table index) or `object:NAME`, or a hex ROM offset.

**extract** writes `OUTDIR/index.json` (every file, its kind and ROM range),
`levels/NN_name/`, `models/name/`, `scenes/name/`, `objects/NNN_name/`
(each `data.bin`, the inflated file, and `level.json` / `model.json` /
`object.json`), the other gzip files (`images/`, `demos/`, `code/`) and the
LZSS files (`lzss/`) decompressed.

**build** reads the JSON files of an extract, writes each file that changed
and leaves the others' compressed bytes as they are (an unchanged extract
rebuilds the original ROM exactly). A changed file goes back in place when it
fits the room up to the next file (compressed at level 6, else level 9, else
level 9 for both members); the ROM keeps every offset, so the game, the
tables and the port need no change, and the header checksums are fixed. A
file that doesn't fit is written for the shiftable build instead
([shiftable](shiftable.md)): `build` writes the compressed file and a pads
JSON (`"rom": {"replace": ...}`), prints the `make SHIFT=1 SHIFT_PADS=...`
command and exits with status 3. The port (bc.exe, bc_headless) loads files
at the original offsets, so a shifted ROM runs in an emulator, not the port.

**gltf** walks the display lists (F3D) the way the RSP draws them and writes
`NAME.gltf`, `NAME.bin` and `textures/*.png`. A level gets its static
geometry plus its buildings (one node per placement; `--no-objects` leaves
them out); `--scale S` sets the units (default 0.01: 100 map units = 1 m).
Not modelled: the vehicles' part matrices (parts come out where their
vertices are, which for the vehicles is assembled), blend modes and fog, the
sky, texture animations.

## Library

```python
import sys; sys.path.insert(0, "tools_port")
from bcfiles import Rom, level, model, rombuild

rom = Rom.load("baserom.us.v11.z64")      # .z64/.v64/.n64; US 1.0 or 1.1
f = rom.level(1)                          # RomFile: gzip members, ROM range, slot
lv = level.Level.parse(f.data, f.split)   # typed sections
lv["objects"][0].x += 256                 # ObjPlacement record
data, split = lv.write()
blob, how = rombuild.pack_to_fit(f, data, split)
```

| module | what |
| --- | --- |
| `romfs` | the ROM walk (gzip members one after another, the LZSS pictures, the scenes, the object table and buildings, the static segment, the code), so a ROM from the shiftable build reads too; the texture and object tables |
| `gz` | gzip members as kiunzip reads them; recompression (the repo's `tools/gzip` when present: byte-identical; else zlib) |
| `lzss` | Rare's LZSS: decoder as `lzss_decode`, and an encoder the decoder accepts |
| `texture` | the seven texture packings, texel formats to RGBA |
| `level` | `Level`: header, segment-8 vertices, every section of levels.md as records, the display-list block |
| `model` | `VehicleModel` (0x4C header: vehicles, markers, front-end scenes), `PackedObject` (0x50 header: buildings) |
| `gbi` | F3D display lists: decoding, a walker |
| `gltf`, `png` | the exporter |
| `rombuild` | in-place replacement, the shiftable-build config, CIC-6102 checksums |
| `mod` | level mods (`.bcmod`, below): parse, check, apply to a `Level` |

Every parser keeps what it can't type: bytes after a section's records are
the section's `tail`, a section that doesn't parse is kept whole (`raw`),
and the padding that only aligns the next section is recomputed. On the US
ROMs nothing needs either: every level, model, scene and building is fully
typed and writes back byte for byte (below).

## Level mods (.bcmod)

bc.exe's level editor (the viewer's Editor panel, `port/README.md`) saves its
work as a mod: a small JSON file with, per level, the SHA-1 of the level file
as the ROM holds it (both gzip members inflated), the changes, and the SHA-1
of the result:

```json
{"format": "bcmod/1", "name": "wide start", "description": "...",
 "levels": [{"level": 1, "original_sha1": "...", "edited_sha1": "...",
             "changes": [{"op": "set", "kind": "object", "index": 0, "fields": {"x": 1880, "z": 1880}},
                         {"op": "add", "kind": "rdu", "index": 100, "copy_of": 99, "fields": {"x": 2000}},
                         {"op": "delete", "kind": "tnt", "index": 3}]}]}
```

Kinds and fields (the record layouts of [levels](systems/levels.md)):
`vehicle` (type, x, y, z, heading), `carrier` (speed, x, z, heading, limit;
one record, no add or delete), `object` (x, y, z, type, path, people,
motion, tag), `rdu` (x, y, z), `tnt` (x, y, z, type, timer, unk8, unkA),
`crate` (x, y, z, type), `commpt` (x, y, z, w), `block` (x, y, z, model),
`slot` (x, y, z, model, target; its triangles move with x, y, z). Changes
apply in order and an index counts the records as they are by then; `add`
inserts a copy of record `copy_of` and sets its fields, so a mod carries no
ROM data beyond the values it sets. Field ranges and the counts the game's
fixed arrays hold (8 vehicle spawns, 55 objects, 400 RDUs, 20 TNT, 15
crates, 2 communication points, 8 blocks, 8 slots with at most 100
triangles), one driver spawn (type 0) and the known vehicle types are
checked; a change that breaks them is refused.

Two implementations build the same bytes: `port/src/load/level_mod.c` (bc.exe,
bc_headless, the editor) and `mod.py` here (both lay the file out as
`Level.write` does: every section at its alignment, the padding recomputed,
the display-list block on 16 bytes with the visibility records' offsets
moved). A mod with `edited_sha1` is refused by either if its result differs.

In the port a mod is applied in memory: `bc.exe --mod FILE` (or bc.ini
`[mods] enabled = name, ...`, files in `mods\`), `bc_headless --mod FILE`
(`--mod-out DIR` writes the edited files for checking). The ROM is never
changed; the game reads the edited file through one hook around
`lvl_load_file`'s `hd_rom_load` (`game/port.h` `PORT_LEVEL_PRE` /
`PORT_LEVEL_POST`). **mod-export** puts a mod's levels into a ROM the way
**build** does: in place when they fit (that ROM runs in bc.exe with
`--no-rom-check` too), else the shiftable build's config (exit status 3; a
shifted ROM runs in emulators only).

## Tests

```bash
python3 tools_port/bcfiles/test_bcfiles.py              # synthetic data only (CI)
python3 tools_port/bcfiles/test_bcfiles.py --rom ROM    # also every file of your ROM
```

The `--rom` run checks, for US 1.1 and 1.0: the 60 levels, 21 models, 3
scenes and 278 buildings parse with nothing left untyped and write back
byte for byte, through JSON as well; recompression with `tools/gzip`
reproduces every member; the six LZSS files decode and survive the encoder;
the texture table decodes; the walk finds every file where the code's
constants put it; an unchanged rebuild is the ROM; an edited level goes back
in place with valid checksums.
