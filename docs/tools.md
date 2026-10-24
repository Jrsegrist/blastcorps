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
python3 tools_port/bcfiles mod-export ROM MOD.bcmod NEW.z64  # a ROM with the mod's levels (and texture slots) in it
python3 tools_port/bcfiles geo-rebuild ROM OUTDIR      # every level's derived sections regenerated (level_NN.bin)
python3 tools_port/bcfiles geo-gltf ROM level:0 OUTDIR # a level's triangles as glTF, for editing in Blender
python3 tools_port/bcfiles geo-import ROM level:0 EDITED.gltf OUT.bcmod   # the edited glTF as a geometry mod
python3 tools_port/bcfiles tex-import ROM 1549 IMAGE.png OUT.bcmod [--into MOD]   # an image into a texture slot
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
| `geometry` | a level's triangles and the generators of everything derived from them (below) |
| `geogltf` | the geometry glTF for Blender and its import as a geometry edit |

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

## Geometry

A level file holds three sets of triangles ([levels](systems/levels.md)):
the drawn ones (the display-list block's lists and the segment-8
vertices), the floor (the ground grid, 0x30: every floor triangle is also a
drawn one, corner for corner, on all 60 US levels) and the walls (collision
grids A and B, 0x6C / 0x70: mostly an invisible mesh of their own).
`geometry.py` takes them out of a level (`Geometry.from_level`) and writes
them back with everything the game derives from them regenerated
(`write_into`):

| derived | rule (what reproduces Rare's data) |
| --- | --- |
| ground grid (0x30) | a floor triangle is in every cell its x-z footprint meets (closed rectangles), except cells lying strictly inside it; every cell lists its triangles in one global order, a topological order of Rare's per-cell lists |
| wall grids (0x6C, 0x70) | the same with every cell grown by 50 map units, half-open (touching the grown rectangle's high edge doesn't count) |
| visibility heights (0x48) | per cell, min(0, lowest y) and max(0, highest y) of the drawn vertices in its closed rectangle |
| bounds (0x4C) | the floor triangles' x-z extent, 50 map units more on every side |
| display lists (0x78..0x84) | each list written again: its state commands in order, its triangles in G_VTX loads of at most 16 vertices (F3D's buffer) between the same vertex-state commands (matrix, texture scale, lighting and texture-generation modes, G_DL), the sub-lists' G_CULLDL box the list's bounding box (Rare's corner order), every offset into the block moved (header 0x7C..0x9C, visibility records and groups, G_DL targets) |

The bridges' packed lists (G_MTX, part matrices, vertices used across a
matrix change) are kept as they are ("frozen"); so are the switch and gate
triangles (0x64, 0x68), the moving-part groups (0x74), the guard triangles
(0x24), the height zones (0x40, 0x44) and every per-level table in the code.

With `base=` (the original level; what mods use) everything an edit
doesn't reach keeps Rare's data: a grid cell whose regenerated list is the
same for both geometries keeps the file's list, likewise visibility cells and
the bounds; a list the edit doesn't touch keeps its commands and vertices; a
list whose triangles only moved keeps its commands and gets its vertices
written over the old ones; vertices no list loads any more are dropped. An
unchanged level comes back byte for byte (all 60).

How close the rules come, rebuilding all 60 US levels from their own
triangles (`geo-rebuild`, no `base`): the draw streams (state commands and
triangles with their vertex values) are equal at all 4,332 entry points;
1,565 of 1,608 ground cells, 1,250 of 1,254 grid-A cells, all grid-B cells,
543 of 545 visibility cells and all 60 bounds equal Rare's. The rest are
triangles that only touch a cell (no area), duplicates, and 11 walls at
exactly 50 units. The game answers the same: loadcheck TB (the game's own
`coll_ground_height` and wall probes, native and the original MIPS in
unicorn, 1,200 random points per level) finds no difference on any level,
TC (native, ~30 million lattice points) none for the ground and 27 wall
probes on levels 41 and 42 (spheres exactly 50 units from a cell), and
bc_headless running verify-levels' input on the 60 rebuilt levels
(`--level-dir`) gives the ROM's traces and game state.

### Editing (`Geometry.apply_edit`, bcmod/2 "geometry")

```json
{"move": [[2243, 123, 2454, 2243, 140, 2454]],
 "remove": {"render": [113], "ground": [683], "walls_a": [], "walls_b": []},
 "add": {"render": [{"after": 114, "v": [[x, y, z, s, t, r, g, b, a], [...], [...]], "flag": 0}],
         "ground": [{"v": [x0, y0, z0, x1, y1, z1, x2, y2, z2], "surface": 1, "flag": 0}],
         "walls_a": [{"v": [nine coordinates], "heading": 0, "side": 0, "fail": 0}]},
 "settimg": [[4, 0, 1434]]}
```

- **move**: every corner at (x, y, z) - drawn, floor or wall - goes to the
  new place (corners are welded by position). A bridge's, moving part's,
  switch's or gate's corner can't be moved.
- **remove**: triangle ids of the unedited level: drawn triangles count in
  the block's order (`Geometry.render_tris()`), floor and wall triangles in
  their global order.
- **add**: drawn triangles go right after triangle `after`, in its list and
  state (its texture, combiner, render mode); not into a frozen list. Floor
  triangles are scanned first in their cells, and Rare's triangles under
  them lose their stop flag: the game's scan stops at the first triangle with
  that flag it hits, and it moves the last triangle a wheel stood on to the
  front of its cell (`ground_cache_tri`). Walls: `side` picks the side that
  blocks (0: the side away from the level's origin), `heading` the facing
  the free arc is cut around (default: the normal's).
- **settimg**: the k-th G_SETTIMG of a list loads another texture id (the
  same texel format and size as the old one, or the picture is garbage).
- Applied in that order: moves, removes, adds (in the moved coordinates),
  settimg. Checked afterwards: the per-frame visible lists against
  `lvl_alloc_dl_buffers`' sizes; the port takes an edited file at most 64 KB
  larger than the ROM's.

Helpers for scripts (and the sculpting tools to come): `subdivide_edit`
(split drawn and floor triangles 4**n ways and raise the new corners by a
height function: hills and dents), `flatten_edit`, `quad_edit` (a ramp or
a platform), `merge_edits`.

`port/src/load/level_geo.c` is the C twin (bc.exe, bc_headless and the
editor apply geometry mods with it when a level loads): loadcheck TD checks
that it rebuilds all 60 levels into the same files as `geo-rebuild` and
gives the ROM's files back keeping Rare's data, and its mod results equal
`mod.py`'s byte for byte.

### glTF for Blender (`geo-gltf`, `geo-import`)

`geo-gltf` writes meshes `render` (every drawn triangle, three vertices of
its own, one primitive per run of triangles sharing a state, material
`r<piece>.<run>`, textured), `ground` (materials `g.s<surface>.f<flag>`) and
`walls_a` / `walls_b`. Every vertex carries where it came from:

| attribute | holds |
| --- | --- |
| TEXCOORD_0 | the texture coordinates (render) |
| TEXCOORD_1 | x, z of the corner in the unedited level (map units) |
| TEXCOORD_2 | y of that corner, the triangle's id |
| TEXCOORD_3 | walls: heading, side * 256 + fail |
| COLOR_0 | the vertex colour (render) |

Material `extras` hold what an added triangle needs (`after`: the run's last
triangle; the texture's size and tile values, to turn UVs back into s, t).
Blender keeps UV maps on face corners, so edit the meshes freely: on import
(`geo-import`, .gltf or .glb) a triangle whose corners carry one id and that
id's original corners is that triangle (corners that moved become moves),
any other triangle is added (into its material's run; floor triangles with
their material's surface and flag; walls with TEXCOORD_3 or the heading of
their normal), and ids that are gone are removed. Positions are rounded to
whole map units (100 map units = 1 m at the default scale 0.01). Moving the
drawn mesh moves the floor too (welded corners); to change where the ground
is without a visible change, edit the `ground` mesh.

### Texture slots (`tex-import`, bcmod/2 "textures")

`tex-import ROM ID IMAGE.png` makes a mod entry `{"id", "fmt", "siz", "png",
"slot_sha1"}`: the slot's texel format is taken from the level lists that use
it (the image must have their size), the image is converted (RGBA16/32,
IA4/8/16, I4/8; colour-indexed slots aren't done) and packed with the slot's
own packing (literals and back-references for packings 1, 2, 3, 6; stored for
0; the palette packings 4 and 5 aren't done). It must fit the slot's room up
to the next table entry (a slot that holds more than the image keeps the
rest). `mod-export` writes the packed bytes and the table entry's size in
place. Adding textures: the texture data is packed back to back (an entry's
DMA length is the next entry's offset minus its own; at most 6 bytes of slack
per entry, 8 bytes after the last one, and the 431 empty entries share their
neighbours' offsets), so there is no room for a new texture in place: use a
slot the edited level doesn't otherwise need, or the shiftable build (which
moves data). The port doesn't apply
texture slots from mods yet (bc.exe logs it); `mod-export` and an emulator,
or `bc.exe --no-rom-check` on an exported ROM that fits, show them.

### Geometry mods (bcmod/2)

```json
{"format": "bcmod/2", "name": "...", "description": "...",
 "levels": [{"level": 0, "original_sha1": "...", "edited_sha1": "...",
             "changes": [...], "geometry": {...}}],
 "textures": [{"id": 1549, "fmt": 0, "siz": 16, "png": "<base64>", "slot_sha1": "..."}]}
```

A mod holds the edit and the author's own values (and images), never ROM
data or anything derived: the record changes apply first, then the geometry,
then every derived section is regenerated (keeping Rare's data where the
edit doesn't reach), then `edited_sha1` is checked. bcmod/1 files load as
before; a bcmod/1 file can't carry geometry. A delta keyed by positions and
ids works for every edit made so far (moves, splits, additions, removals,
retextures); what it can't express is changing a list's state commands
beyond a texture id (render modes, combiners): that would need its own op.

`--level-dir DIR` (bc_headless, bc.exe; testing) makes the game read
`DIR\level_NN.bin` (inflated, the ROM's byte order: `geo-rebuild`,
`mod-apply`) instead of the ROM's level through the same hook.

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
