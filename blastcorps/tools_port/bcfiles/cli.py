"""bcfiles command line (python3 -m bcfiles, with tools_port on PYTHONPATH, or
python3 tools_port/bcfiles).

  list ROM                      the ROM's files and their kinds
  dump ROM FILE [--json]        a level, model, scene or building as text (or JSON)
  extract ROM OUTDIR            every file decompressed, levels/models/buildings also as JSON
  build ROM JSONDIR OUT.z64     a ROM with the edited JSON files of an extract put back
  gltf ROM FILE OUTDIR          glTF 2.0 (+ PNG textures) of a model, building or level
  roundtrip ROM                 parse and write every level/model/scene/building, compare
  mod-check ROM MOD.bcmod       a level mod (bc.exe's editor): its hashes and changes against the ROM
  mod-apply ROM MOD.bcmod OUTDIR   the edited level files (level_NN.bin, inflated)
  mod-export ROM MOD.bcmod OUT.z64 [--shift-dir D]   a ROM with the mod's levels in it (for emulators)

FILE: level:N | level:NAME | model:NAME | scene:NAME | object:N (object-table index)
| object:NAME | a hex ROM offset (0x4A5660).
"""
import json
import os
import sys

from . import gz, lzss, rombuild
from .level import Level
from .model import PackedObject, VehicleModel
from .romfs import Rom

PARSERS = {"level": Level, "model": VehicleModel, "scene": VehicleModel, "object": PackedObject}


def usage():
    sys.stderr.write(__doc__)
    sys.exit(1)


def find(rom, spec):
    kind, _, key = spec.partition(":")
    files = rom.files()
    if not key:
        try:
            off = int(spec, 16)
        except ValueError:
            raise SystemExit("bad FILE %r" % spec)
        for f in files:
            if f.start == off:
                return f
        raise SystemExit("no file starts at %06X" % off)
    if kind == "level" and key.isdigit():
        return rom.level(int(key))
    if kind == "object" and key.isdigit():
        return rom.object(int(key))
    for f in files:
        if f.kind == kind and f.name == key:
            if kind == "level":
                return rom.level(f.info["level"])
            return f
    raise SystemExit("no %s named %r" % (kind, key))


def parse_file(f):
    cls = PARSERS[f.kind]
    return cls.parse(f.data, f.split)


# ------------------------------------------------------------------- list

def cmd_list(args):
    rom = Rom.load(args[0])
    print("%s (%s), compressor for edits: %s" % (args[0], rom.version, gz.which_compressor()))
    print("%-7s %-8s %-8s %-28s %s" % ("kind", "start", "end", "name", "info"))
    for f in rom.files():
        info = []
        if f.kind == "level":
            info.append("level %d" % f.info["level"])
        if f.kind == "model":
            info.append("types " + ",".join("0x%X" % t for t in f.info["types"]))
        if f.kind == "object":
            idx = f.info.get("indexes", [])
            info.append("table %s" % (",".join(map(str, idx[:6])) + ("..." if len(idx) > 6 else "")))
        if f.members:
            info.append("gzip x%d -> %d bytes" % (len(f.members), len(f.data)))
        if f.kind == "lzss":
            info.append("lzss %d-bit" % f.info["bits"])
        print("%-7s %06X   %06X   %-28s %s" % (f.kind, f.start, f.end, f.name[:28], "; ".join(info)))


# ------------------------------------------------------------------- dump

def _fmt_rec(r):
    if hasattr(r, "to_json"):
        return " ".join("%s=%s" % kv for kv in r.to_json().items())
    return str(r)


def cmd_dump(args):
    as_json = "--json" in args
    full = "--all" in args
    args = [a for a in args if not a.startswith("--")]
    rom = Rom.load(args[0])
    f = find(rom, args[1])
    p = parse_file(f)
    if as_json:
        json.dump(p.to_json(), sys.stdout, indent=1)
        print()
        return
    print("%s %s: ROM %06X-%06X, %d bytes inflated (%s)" % (f.kind, f.name, f.start, f.end, len(f.data),
                                                         " + ".join(str(len(m.data)) for m in f.members)))
    for m in f.members:
        print("  gzip member %s mtime %d at %06X" % (m.name, m.mtime, m.start))
    for n in p.notes:
        print("  note:", n)
    if getattr(p, "raw", None) is not None:
        print("  (kept as raw bytes)")
        return
    if isinstance(p, Level):
        print("header:", " ".join("%s=%s" % kv for kv in p.header.items()))
        print("vertices: %d" % len(p.vertices))
        bulk = {"guard_tris", "ground_grid", "coll_grid_a", "coll_grid_b", "vis_grid", "vis_records_0",
                "vis_records_1", "vis_records_2", "vis_records_3", "vis_groups_0", "vis_groups_1", "vis_groups_2",
                "vis_groups_3", "part_groups", "switch_groups", "gate_groups"}
        summ = p.summary()
        for s, sec in p.sections.items():
            v = sec.value
            print("[0x%02X %s] %s" % (s, sec.name, summ.get(sec.name)))
            if sec.name in bulk and not full:
                continue
            if isinstance(v, list):
                for i, r in enumerate(v):
                    print("   %3d %s" % (i, _fmt_rec(r) if not isinstance(r, dict) else
                                         json.dumps({k: (x.to_json() if hasattr(x, "to_json") else x) for k, x in r.items()})))
            elif v is not None and hasattr(v, "to_json"):
                print("   ", _fmt_rec(v))
            elif isinstance(v, dict):
                print("   ", json.dumps(json.loads(json.dumps(v, default=lambda o: o.to_json())))[:2000])
        print("display lists: %d bytes; entry points %s" % (len(p.dl), " ".join("0x%02X=+%X" % kv for kv in p.dl_ptrs.items())))
    else:
        if isinstance(p, PackedObject):
            print("header:", " ".join("%s=%s" % kv for kv in p.fields.items()))
            print("vertices: %d" % len(p.vertices))
        for k, v in p.summary().items():
            print("  %-14s %s" % (k, v))
        if isinstance(p, VehicleModel):
            print("  dl entries:", " ".join("%s=+%X" % kv for kv in p.dl_entries().items()))
        tex = set()
        dl = p.dl_bytes() or b""
        for q in range(0, len(dl) - 7, 8):
            if dl[q] == 0xFD:
                tex.add(int.from_bytes(dl[q + 4:q + 8], "big"))
        print("  textures (table ids): %s" % " ".join(str(t) for t in sorted(tex)))
        if full:
            from . import gbi
            for q in range(0, len(dl) - 7, 8):
                w0, w1 = int.from_bytes(dl[q:q + 4], "big"), int.from_bytes(dl[q + 4:q + 8], "big")
                print("    +%05X %s" % (q, gbi.describe(w0, w1)))


# ---------------------------------------------------------------- extract

def file_dir(f):
    if f.kind == "level":
        return os.path.join("levels", "%02d_%s" % (f.info["level"], f.name))
    if f.kind == "object":
        return os.path.join("objects", "%03d_%s" % (f.info["indexes"][0], f.name))
    return os.path.join(f.kind + "s", f.name)


def cmd_extract(args):
    rom = Rom.load(args[0])
    out = args[1]
    os.makedirs(out, exist_ok=True)
    index = {"format": "bcfiles.extract/1", "version": rom.version, "files": []}
    n = 0
    for f in rom.files():
        ent = {"kind": f.kind, "name": f.name, "start": "0x%06X" % f.start, "end": "0x%06X" % f.end}
        if f.kind in PARSERS:
            d = file_dir(f)
            if f.kind == "level":
                f = rom.level(f.info["level"])
            os.makedirs(os.path.join(out, d), exist_ok=True)
            with open(os.path.join(out, d, "data.bin"), "wb") as fh:
                fh.write(f.data)
            p = parse_file(f)
            j = p.to_json()
            j["rom"] = {"start": "0x%06X" % f.start, "kind": f.kind, "name": f.name,
                        "members": [m.meta_json() for m in f.members]}
            with open(os.path.join(out, d, f.kind + ".json"), "w") as fh:
                json.dump(j, fh, indent=0)
            ent["dir"] = d
        elif f.members:
            d = {"image": "images", "demos": "demos", "code": "code"}.get(f.kind, f.kind + "s")
            os.makedirs(os.path.join(out, d), exist_ok=True)
            names = []
            for i, m in enumerate(f.members):
                nm = "%06X_%s%s.bin" % (f.start, f.name, "" if len(f.members) == 1 else "_%d" % i)
                with open(os.path.join(out, d, nm), "wb") as fh:
                    fh.write(m.data)
                names.append(os.path.join(d, nm))
            ent["files"] = names
        elif f.kind == "lzss":
            data, end = lzss.decompress(rom.data, f.info["bits"], f.start)
            nm = os.path.join("lzss", "%06X.bin" % f.start)
            os.makedirs(os.path.join(out, "lzss"), exist_ok=True)
            with open(os.path.join(out, nm), "wb") as fh:
                fh.write(data)
            ent["files"] = [nm]
            ent["lzss_end"] = "0x%06X" % end
        index["files"].append(ent)
        n += 1
    with open(os.path.join(out, "index.json"), "w") as fh:
        json.dump(index, fh, indent=1)
    print("extract: %d files into %s (index.json)" % (n, out))


# ------------------------------------------------------------------ build

def cmd_build(args):
    shift_dir = None
    if "--shift-dir" in args:
        i = args.index("--shift-dir")
        shift_dir = args[i + 1]
        del args[i:i + 2]
    rom = Rom.load(args[0])
    src, out = args[1], args[2]
    patches = []
    for f in rom.files():
        if f.kind not in PARSERS:
            continue
        if f.kind == "level":
            f = rom.level(f.info["level"])
        path = os.path.join(src, file_dir(f), f.kind + ".json")
        if not os.path.exists(path):
            continue
        with open(path) as fh:
            j = json.load(fh)
        p = PARSERS[f.kind].from_json(j)
        data, split = p.write()
        changed = data != f.data or split != f.split
        if not changed:
            continue
        blob, how = rombuild.pack_to_fit(f, data, split)
        pt = rombuild.Patch(f, blob, True, how)
        print("build: %s %s changed: %d -> %d bytes inflated, %d -> %d compressed (%s; slot %d): %s"
              % (f.kind, f.name, len(f.data), len(data), f.end - f.start, len(blob), how, pt.slot,
                 "in place" if pt.fits else "does NOT fit"))
        patches.append(pt)
    image, rest = rombuild.apply_in_place(rom.data, patches)
    if rest:
        d = shift_dir or (out + ".shift")
        cfg = rombuild.shift_config(rest, d)
        print("build: %d file(s) grew past their slot; for the shiftable build (docs/shiftable.md):" % len(rest))
        print("  make VERSION=us.v11 SHIFT=1 BASEROM=<this ROM> SHIFT_PADS=%s" % os.path.abspath(cfg))
        print("  (%s not written)" % out)
        sys.exit(3)
    with open(out, "wb") as fh:
        fh.write(image)
    print("build: %s written (%d file(s) replaced in place, compressor: %s)" % (out, len(patches), gz.which_compressor()))


# ------------------------------------------------------------------- gltf

def cmd_gltf(args):
    from . import gltf
    scale = 0.01
    if "--scale" in args:
        i = args.index("--scale")
        scale = float(args[i + 1])
        del args[i:i + 2]
    no_obj = "--no-objects" in args
    args = [a for a in args if a != "--no-objects"]
    rom = Rom.load(args[0])
    f = find(rom, args[1])
    p = parse_file(f)
    name = f.name
    if f.kind == "level":
        path, st = gltf.export_level(rom, p, args[2], "level%02d_%s" % (f.info["level"], name), not no_obj, scale)
    elif f.kind == "object":
        path, st = gltf.export_object(rom, p, args[2], name, scale)
    else:
        path, st = gltf.export_model(rom, p, args[2], name, scale)
    print("gltf: %s (%s)" % (path, ", ".join("%s %s" % kv for kv in st.items())))


# -------------------------------------------------------------- roundtrip

def roundtrip(rom, verbose=False):
    """(results per kind, [failures]) for every level, model, scene and building"""
    res = {}
    bad = []
    files = [rom.level(n) for n in range(60)] + [f for f in rom.files() if f.kind in ("model", "scene", "object")]
    for f in files:
        cls = PARSERS[f.kind]
        p = cls.parse(f.data, f.split)
        d, s = p.write()
        ok_bytes = d == f.data and s == f.split
        j = json.loads(json.dumps(p.to_json()))
        p2 = cls.from_json(j)
        d2, s2 = p2.write()
        ok_json = (d2, s2) == (d, s) and json.loads(json.dumps(p2.to_json())) == j
        # recompression: the members rebuilt from the inflated data
        blob = b"".join(m.header() + gz.compress(m.data) + m.trailer() for m in f.members)
        orig = b"".join(m.build() for m in f.members)
        r = res.setdefault(f.kind, {"files": 0, "bytes_equal": 0, "json_equal": 0, "typed": 0, "fully_typed": 0,
                                    "raw_bytes": 0, "recompress_equal": 0})
        r["files"] += 1
        r["bytes_equal"] += ok_bytes
        r["json_equal"] += ok_json
        r["typed"] += getattr(p, "raw", None) is None
        r["fully_typed"] += p.unexplained() == 0
        r["raw_bytes"] += p.unexplained()
        r["recompress_equal"] += blob == orig
        if not (ok_bytes and ok_json):
            bad.append("%s %s: bytes %s json %s" % (f.kind, f.name, ok_bytes, ok_json))
        if verbose and p.notes:
            print("  %s %s: %s" % (f.kind, f.name, "; ".join(p.notes)))
    return res, bad


def cmd_roundtrip(args):
    rom = Rom.load(args[0])
    res, bad = roundtrip(rom, verbose=True)
    print("roundtrip %s (%s; compressor %s):" % (args[0], rom.version, gz.which_compressor()))
    for k, r in res.items():
        print("  %-7s %s" % (k, " ".join("%s=%d" % kv for kv in r.items())))
    for b in bad:
        print("  FAIL", b)
    print("roundtrip: %s" % ("OK" if not bad else "FAILED"))
    sys.exit(1 if bad else 0)


# -------------------------------------------------------------------- mods

def _load_mod(rom, path):
    from . import mod
    try:
        m = mod.load(path)
        return m, mod.apply_mod(rom, m)
    except (mod.ModError, ValueError) as e:
        raise SystemExit("%s: %s" % (path, e))


def cmd_mod_check(args):
    from . import mod
    rom = Rom.load(args[0])
    m, out = _load_mod(rom, args[1])
    print("%s: \"%s\" (%s)" % (args[1], m.get("name", ""), m.get("description", "")))
    for ml in m["levels"]:
        data, split = out[ml["level"]]
        f = rom.level(ml["level"])
        print("  level %d (%s): %d change%s, %d -> %d bytes inflated, edited SHA-1 %s%s" % (
            ml["level"], f.name, len(ml["changes"]), "" if len(ml["changes"]) == 1 else "s", len(f.data), len(data),
            mod.sha1(data), " (as the mod says)" if ml.get("edited_sha1") else ""))
        for c in ml["changes"]:
            print("    " + mod.describe(c))
    print("mod-check: OK")


def cmd_mod_apply(args):
    rom = Rom.load(args[0])
    _, out = _load_mod(rom, args[1])
    os.makedirs(args[2], exist_ok=True)
    for n, (data, split) in sorted(out.items()):
        path = os.path.join(args[2], "level_%02d.bin" % n)
        with open(path, "wb") as fh:
            fh.write(data)
        print("mod-apply: %s (%d bytes, split 0x%X)" % (path, len(data), split))


def cmd_mod_export(args):
    shift_dir = None
    if "--shift-dir" in args:
        i = args.index("--shift-dir")
        shift_dir = args[i + 1]
        del args[i:i + 2]
    rom = Rom.load(args[0])
    _, out = _load_mod(rom, args[1])
    patches = []
    for n, (data, split) in sorted(out.items()):
        f = rom.level(n)
        if data == f.data:
            continue
        blob, how = rombuild.pack_to_fit(f, data, split)
        pt = rombuild.Patch(f, blob, True, how)
        print("mod-export: level %d %s: %d -> %d bytes inflated, %d -> %d compressed (%s; slot %d): %s"
              % (n, f.name, len(f.data), len(data), f.end - f.start, len(blob), how, pt.slot,
                 "in place" if pt.fits else "does NOT fit"))
        patches.append(pt)
    image, rest = rombuild.apply_in_place(rom.data, patches)
    if rest:
        d = shift_dir or (args[2] + ".shift")
        cfg = rombuild.shift_config(rest, d)
        print("mod-export: %d level(s) grew past their slot; for the shiftable build (docs/shiftable.md):" % len(rest))
        print("  make VERSION=us.v11 SHIFT=1 BASEROM=<this ROM> SHIFT_PADS=%s" % os.path.abspath(cfg))
        print("  (%s not written; a ROM with files moved runs in an emulator, not in bc.exe: use the mod there)"
              % args[2])
        sys.exit(3)
    with open(args[2], "wb") as fh:
        fh.write(image)
    print("mod-export: %s written (%d level(s) in place; bc.exe needs --no-rom-check for it, or use the mod)"
          % (args[2], len(patches)))


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    if not argv:
        usage()
    cmd, args = argv[0], argv[1:]
    table = {"list": (cmd_list, 1), "dump": (cmd_dump, 2), "extract": (cmd_extract, 2), "build": (cmd_build, 3),
             "gltf": (cmd_gltf, 3), "roundtrip": (cmd_roundtrip, 1), "mod-check": (cmd_mod_check, 2),
             "mod-apply": (cmd_mod_apply, 3), "mod-export": (cmd_mod_export, 3)}
    if cmd not in table or len([a for a in args if not a.startswith("--")]) < table[cmd][1]:
        usage()
    table[cmd][0](args)
