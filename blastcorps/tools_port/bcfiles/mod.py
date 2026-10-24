"""Level mods (.bcmod): the change lists bc.exe's level editor saves.

A mod lists, per level, the SHA-1 of the level file as the ROM holds it
(both gzip members inflated back to back) and a list of changes to its
records; applied in order, they give the edited file (and its SHA-1, when the
mod has one).  bc.exe and bc_headless apply mods in memory
(port/src/load/level_mod.c); this module applies them here, with the same
rules and the same layout (Level.write), so a mod can also go into a ROM
for an emulator (`bcfiles mod-export`).

    {"format": "bcmod/1", "name": "...", "description": "...",
     "levels": [{"level": 1, "original_sha1": "...", "edited_sha1": "...",
                 "changes": [{"op": "set", "kind": "object", "index": 0, "fields": {"x": 1880}},
                             {"op": "add", "kind": "rdu", "index": 100, "copy_of": 99, "fields": {"x": 2000}},
                             {"op": "delete", "kind": "tnt", "index": 3}]}]}

An "add" inserts a copy of record copy_of (counted before the insert) at
index and then sets its fields, so a mod holds no ROM data beyond the field
values it sets.

"bcmod/2" (bcmod/1 files still load) adds, per level, "geometry": an edit of
the level's triangles (geometry.Geometry.apply_edit: moved corners, removed
triangle ids, added triangles with the values the mod's author gives, other
texture ids for a list's G_SETTIMG), and at the top "textures": texture-table
slots with the author's own image (a PNG, base64) and the SHA-1 of the slot's
packed bytes in the ROM it was made for.  Nothing derived is stored: the ground
and wall grids, the visibility heights, the bounds and the display lists are
regenerated from the edited triangles when the mod is applied (keeping Rare's
data wherever the edit doesn't reach).
"""
import hashlib
import json

from .binio import Record
from .level import Level

# kind -> (section, record size, the most records the game's arrays hold, fields: (name, lo, hi))
S16 = (-32768, 32767)
KINDS = {
    "vehicle": (0x50, 9, 8, [("type", 0, 18), ("x",) + S16, ("y",) + S16, ("z",) + S16, ("heading", 0, 4095)]),
    "carrier": (0x54, 9, 1, [("speed", 0, 255), ("x",) + S16, ("z",) + S16, ("heading", 0, 4095), ("limit",) + S16]),
    "object": (0x5C, 14, 55, [("x", 0, 65535), ("y", 0, 65535), ("z", 0, 65535), ("type", 0, 511), ("path", 0, 255),
                              ("people", 0, 255), ("motion", 0, 5), ("tag", 0, 65535)]),
    "rdu": (0x34, 6, 400, [("x",) + S16, ("y",) + S16, ("z",) + S16]),
    "tnt": (0x38, 12, 20, [("x",) + S16, ("y",) + S16, ("z",) + S16, ("type", 0, 1), ("timer", 0, 255), ("unk8",) + S16,
                           ("unkA",) + S16]),
    "crate": (0x20, 8, 15, [("x",) + S16, ("y",) + S16, ("z",) + S16, ("type", 0, 1)]),
    "commpt": (0x28, 8, 2, [("x",) + S16, ("y",) + S16, ("z",) + S16, ("w", 0, 65535)]),
    "block": (0x3C, 8, 8, [("x",) + S16, ("y",) + S16, ("z",) + S16, ("model", 0, 2)]),
    "slot": (0x3C, 10, 8, [("x",) + S16, ("y",) + S16, ("z",) + S16, ("model", 0, 2), ("target",) + S16]),
}
KIND_ORDER = ["vehicle", "carrier", "object", "rdu", "tnt", "crate", "commpt", "block", "slot"]
MAX_SLOT_TRIS = 100


class ModError(Exception):
    pass


def sha1(data):
    return hashlib.sha1(bytes(data)).hexdigest()


# ------------------------------------------------------------------ records of a parsed level

def _records(lv, kind):
    """the kind's record list in the Level (a list the caller may change); None: not editable here"""
    slot = KINDS[kind][0]
    sec = lv.sections.get(slot)
    if sec is None or sec.raw is not None:
        return None
    if kind == "carrier":
        return None if sec.value is None else [sec.value]
    if kind in ("block", "slot"):
        if sec.value is None:
            return None
        return sec.value["blocks" if kind == "block" else "slots"]
    return sec.value


def count(lv, kind):
    r = _records(lv, kind)
    return len(r) if r is not None else 0


def get(lv, kind, index, field):
    r = _records(lv, kind)[index]
    return r[field] if isinstance(r, dict) else getattr(r, field)


def _set(lv, kind, index, field, value):
    recs = _records(lv, kind)
    if recs is None or not 0 <= index < len(recs):
        raise ModError("no %s %d" % (kind, index))
    spec = dict((f[0], f[1:]) for f in KINDS[kind][3])
    if field not in spec:
        raise ModError("%s has no field %s" % (kind, field))
    lo, hi = spec[field]
    if not isinstance(value, int) or isinstance(value, bool) or not lo <= value <= hi:
        raise ModError("%s %s: %r is outside %d..%d" % (kind, field, value, lo, hi))
    r = recs[index]
    if kind == "slot":
        if field in ("x", "y", "z"):
            # the slot's triangles move with it
            d = value - r[field]
            names = [field + str(v) for v in range(3)]
            for t in r["tris"]:
                for n in names:
                    if not -32768 <= getattr(t, n) + d <= 32767:
                        raise ModError("slot %d: moving it puts a triangle outside the s16 range" % index)
            for t in r["tris"]:
                for n in names:
                    setattr(t, n, getattr(t, n) + d)
        r[field] = value
    else:
        setattr(r, field, value)


def _clone(v):
    """a copy of a record, a slot (dict) or a list of them (Records copy by their values)"""
    if isinstance(v, Record):
        return type(v)(*list(v._v))
    if isinstance(v, dict):
        return dict((k, _clone(x)) for k, x in v.items())
    if isinstance(v, list):
        return [_clone(x) for x in v]
    return v


def apply_change(lv, ch):
    """one change (a dict of the mod's "changes"); the level is left as it was on an error"""
    op, kind, index = ch.get("op"), ch.get("kind"), ch.get("index")
    if kind not in KINDS or op not in ("set", "add", "delete") or not isinstance(index, int):
        raise ModError("change %r: op, kind or index not known" % (ch,))
    fields = ch.get("fields") or {}
    sec = lv.sections[KINDS[kind][0]]
    before = (_clone(sec.value), sec.tail, sec.raw)
    try:
        recs = _records(lv, kind)
        if op == "add":
            if kind == "carrier":
                raise ModError("a level has one carrier record: it can't be added")
            src = ch.get("copy_of")
            if recs is None or not isinstance(src, int) or not 0 <= src < len(recs) or not 0 <= index <= len(recs):
                raise ModError("add %s at %s as a copy of %s: no such record" % (kind, index, src))
            if len(recs) >= KINDS[kind][2]:
                raise ModError("a level holds at most %d %s records (the game's arrays)" % (KINDS[kind][2], kind))
            recs.insert(index, _clone(recs[src]))
        elif op == "delete":
            if kind == "carrier":
                raise ModError("the carrier record can't be deleted (set its speed to 0: no carrier)")
            if recs is None or not 0 <= index < len(recs):
                raise ModError("no %s %d" % (kind, index))
            del recs[index]
        if op != "delete":
            for name, value in fields.items():
                _set(lv, kind, index, name, value)
    except ModError:
        sec.value, sec.tail, sec.raw = before
        raise


def check_rules(lv):
    """the rules the game's set-up relies on (level_mod.c lm_check)"""
    for kind in KIND_ORDER:
        if count(lv, kind) > KINDS[kind][2]:
            raise ModError("%d %s records: the game holds at most %d" % (count(lv, kind), kind, KINDS[kind][2]))
    veh = _records(lv, "vehicle")
    drivers = 0
    for i in range(count(lv, "vehicle")):
        t = veh[i].type
        drivers += t == 0
        if t == 12 or t > 18:
            raise ModError("vehicle spawn %d: type %d has no vehicle (the game stops its list there)" % (i, t))
    if veh is not None and drivers == 0:
        raise ModError("no driver spawn (type 0): the game looks for one without an end")
    slots = _records(lv, "slot") or []
    tris = sum(len(s["tris"]) for s in slots)
    if tris > MAX_SLOT_TRIS:
        raise ModError("the slots have %d triangles: the game holds at most %d" % (tris, MAX_SLOT_TRIS))


def describe(ch):
    head = "%s %s %d" % (ch["op"], ch["kind"], ch["index"])
    if ch["op"] == "add":
        head += " (copy of %d)" % ch["copy_of"]
    f = ch.get("fields") or {}
    return head + (": " + ", ".join("%s %s" % kv for kv in f.items()) if f and ch["op"] != "delete" else "")


# ------------------------------------------------------------------ mod files

def load(path):
    with open(path, "rb") as fh:
        text = fh.read()
    if text.startswith(b"\xef\xbb\xbf"):
        text = text[3:]
    return parse(json.loads(text.decode("utf-8")))


FORMATS = ("bcmod/1", "bcmod/2")


def parse(j):
    if not isinstance(j, dict) or j.get("format") not in FORMATS:
        raise ModError("not a Blast Corps mod (\"format\": \"bcmod/1\" or \"bcmod/2\" missing)")
    v2 = j["format"] == "bcmod/2"
    for t in j.get("textures", []) if v2 else []:
        if not isinstance(t, dict) or not isinstance(t.get("id"), int) or not isinstance(t.get("png"), str) \
                or not isinstance(t.get("slot_sha1"), str):
            raise ModError("\"textures\": {\"id\", \"png\" (base64), \"slot_sha1\"} expected")
    if not v2 and ("textures" in j or any("geometry" in L for L in j.get("levels", []) if isinstance(L, dict))):
        raise ModError("geometry and textures need \"format\": \"bcmod/2\"")
    levels = j.get("levels")
    if not isinstance(levels, list):
        raise ModError("\"levels\": a list expected")
    seen = set()
    for L in levels:
        n = L.get("level")
        if not isinstance(n, int) or not 0 <= n <= 59:
            raise ModError("\"level\" (0..59) expected")
        if n in seen:
            raise ModError("level %d is listed twice" % n)
        seen.add(n)
        for key in ("original_sha1", "edited_sha1"):
            v = L.get(key)
            if v is None and key == "edited_sha1":
                continue
            if not isinstance(v, str) or len(v) != 40 or any(c not in "0123456789abcdefABCDEF" for c in v):
                raise ModError("level %d: %s: 40 hex digits expected" % (n, key))
            L[key] = v.lower()
        if "changes" not in L and "geometry" in L:
            L["changes"] = []
        if not isinstance(L.get("changes"), list):
            raise ModError("level %d: \"changes\": a list expected" % n)
        if "geometry" in L and not isinstance(L["geometry"], dict):
            raise ModError("level %d: \"geometry\": an object expected" % n)
        for c in L["changes"]:
            if c.get("kind") not in KINDS or c.get("op") not in ("set", "add", "delete"):
                raise ModError("level %d: change %r not known" % (n, c))
            if c["op"] == "add" and not isinstance(c.get("copy_of"), int):
                raise ModError("level %d: an add needs \"copy_of\"" % n)
            for k, v in (c.get("fields") or {}).items():
                if k not in dict((f[0], 1) for f in KINDS[c["kind"]][3]) or not isinstance(v, int):
                    raise ModError("level %d: field %r of %s (a whole number) not known" % (n, k, c["kind"]))
    return j


def apply_level(ml, data, split):
    """one level of a mod over the ROM's file -> (data, split) of the edited file"""
    n = ml["level"]
    if sha1(data) != ml["original_sha1"]:
        raise ModError("level %d: made for another level file (SHA-1 %s, this ROM's is %s)"
                       % (n, ml["original_sha1"], sha1(data)))
    lv = Level.parse(data, split)
    if lv.raw is not None:
        raise ModError("level %d: the file doesn't parse (%s)" % (n, "; ".join(lv.notes)))
    for i, c in enumerate(ml["changes"]):
        try:
            apply_change(lv, c)
        except ModError as e:
            raise ModError("level %d change %d (%s): %s" % (n, i, describe(c), e))
    try:
        check_rules(lv)
    except ModError as e:
        raise ModError("level %d: %s" % (n, e))
    if ml.get("geometry"):
        from . import geometry
        try:
            g = geometry.Geometry.from_level(lv)
            g.apply_edit(ml["geometry"], lv)
            g.write_into(lv, base=Level.parse(data, split))
            problems = g.check_budgets(lv, n)
        except geometry.GeometryError as e:
            raise ModError("level %d geometry: %s" % (n, e))
        if problems:
            raise ModError("level %d geometry: %s" % (n, "; ".join(problems)))
    out, s = lv.write()
    if ml.get("edited_sha1") and sha1(out) != ml["edited_sha1"]:
        raise ModError("level %d: the edited file came out with SHA-1 %s, the mod says %s"
                       % (n, sha1(out), ml["edited_sha1"]))
    return out, s


def apply_mod(rom, mod):
    """{level: (data, split)} for every level of the mod, over the ROM (romfs.Rom)"""
    out = {}
    for ml in mod["levels"]:
        f = rom.level(ml["level"])
        out[ml["level"]] = apply_level(ml, f.data, f.split)
    return out


def make(name, description, levels, textures=None):
    """a mod: levels = [(level, original file, changes, edited file or None[, geometry edit])];
    textures: [{"id", "png", "slot_sha1"}] (bcmod/2)"""
    v2 = bool(textures) or any(len(t) > 4 and t[4] for t in levels)
    j = {"format": "bcmod/2" if v2 else "bcmod/1", "name": name, "description": description, "levels": []}
    for t in levels:
        n, orig, changes, edited = t[:4]
        L = {"level": n, "original_sha1": sha1(orig)}
        if edited is not None:
            L["edited_sha1"] = sha1(edited)
        L["changes"] = changes
        if len(t) > 4 and t[4]:
            L["geometry"] = t[4]
        j["levels"].append(L)
    if textures:
        j["textures"] = textures
    return j


# ------------------------------------------------------------------ textures (bcmod/2)

def texture_patches(rom, mod):
    """[(ROM offset, packed bytes, table entry index, new packed size)] for the
    mod's texture slots, checked against the ROM (slot_sha1) and its room"""
    import base64
    from . import png, texture
    out = []
    ents = rom.tex_table
    for t in mod.get("textures", []):
        i = t["id"]
        if not 0 <= i < texture.TEX_ENTRIES:
            raise ModError("texture %d: no such table entry" % i)
        e = ents[i]
        slot = rom.data[e.rom:e.rom + e.size]
        if sha1(slot) != t["slot_sha1"].lower():
            raise ModError("texture %d: made for another texture (SHA-1 %s, this ROM's is %s)"
                           % (i, t["slot_sha1"], sha1(slot)))
        try:
            w, h, rgba = png.decode_rgba(base64.b64decode(t["png"]))
        except Exception as ex:
            raise ModError("texture %d: the PNG doesn't read (%s)" % (i, ex))
        fmt, siz = t.get("fmt"), t.get("siz")
        if fmt is None or siz is None:
            raise ModError("texture %d: \"fmt\" and \"siz\" (the texel format the level's lists use) expected" % i)
        try:
            packed = texture.encode_slot(rgba, w, h, fmt, siz, e.type, rom.texture(i))
        except texture.TextureError as ex:
            raise ModError("texture %d: %s" % (i, ex))
        if len(packed) > e.dma_len:
            raise ModError("texture %d: the image packs to %d bytes, the slot has room for %d (try fewer colours "
                           "or a smaller image)" % (i, len(packed), e.dma_len))
        out.append((e.rom, packed, i, len(packed)))
    return out


def dumps(mod):
    return json.dumps(mod, indent=1) + "\n"
