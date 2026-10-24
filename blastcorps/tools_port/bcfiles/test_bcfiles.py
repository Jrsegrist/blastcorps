"""bcfiles tests.

    python3 tools_port/bcfiles/test_bcfiles.py              synthetic data only (no ROM; CI)
    python3 tools_port/bcfiles/test_bcfiles.py --rom ROM    also every file of the player's ROM:
        parse/write/JSON round trip of every level, model, scene and building,
        recompression, LZSS, the texture table, an unchanged rebuild == the ROM

Nothing here contains or writes ROM data: the --rom tests read the ROM given
and compare in memory.
"""
import json
import os
import random
import struct
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path[:] = [p for p in sys.path if os.path.abspath(p or ".") != HERE]
sys.path.insert(0, os.path.dirname(HERE))

from bcfiles import gz, lzss, png, rombuild, texture  # noqa: E402
from bcfiles.binio import Vtx  # noqa: E402
from bcfiles import level as L  # noqa: E402
from bcfiles import model as M  # noqa: E402

ROM_PATH = None


# ------------------------------------------------------------------ helpers

def synthetic_level(nobj=3):
    lv = L.Level()
    lv.header = {k: 0 for k in L.HEADER_U16}
    lv.header.update(vis_cols=2, vis_rows=1, ground_cols=1, ground_rows=2, coll_cols=2, coll_rows=1,
                     gravity=-4, dl_profile=1)
    lv.vertices = [Vtx(i, 2 * i, -i, 0, 32 * i, 0, 255, 128, 0, 255) for i in range(6)]
    t9 = [10, 0, 10, 20, 0, 10, 10, 0, 30]
    sec = {}
    sec[0x20] = [L.Crate(1, 2, 3, 0)]
    sec[0x24] = [L.GuardTri(*(t9 + [7]))]
    sec[0x28] = [L.CommPoint(5, 6, 7, 1)]
    sec[0x2C] = [{"head": L.TexAnimHead(9, 3, 0, 1, 0, 4, 0), "ids": [11, 12]}]
    sec[0x30] = [[L.GroundTri(*(t9 + [1, 0]))], []]
    sec[0x34] = [L.Rdu(1, 1, 1), L.Rdu(2, 2, 2)]
    sec[0x38] = [L.Tnt(1, 2, 3, 0, 5, 0, 0)]
    sec[0x3C] = {"blocks": [L.Block(1, 2, 3, 0)],
                 "slots": [{"x": 4, "y": 5, "z": 6, "model": 0, "target": 1, "tris": [L.CollTri(*(t9 + [0x400, 1, 0]))]}]}
    sec[0x40] = [L.HeightZone(0, 0, 100, 100, 3000)]
    sec[0x44] = []
    sec[0x48] = [[0, 10], [5, 20]]
    sec[0x4C] = L.Bounds(-100, -100, 100, 100)
    sec[0x50] = [L.VehSpawn(0, 10, 0, 10, 0), L.VehSpawn(1, 10, 0, 10, 0x400)]
    sec[0x54] = L.Carrier(3, 0, -500, 0, 2000)
    sec[0x58] = [L.PanelEvent(100, 1, 2, 3, 4, 0, 0)]
    sec[0x5C] = [L.ObjPlacement(100 * i, 0, 50, i, 0, 2, 0, 0) for i in range(nobj)]
    sec[0x60] = {"default": 255, "zones": [{"x": 1, "y": 2, "z": 3, "radius": 50, "mode": 1, "vehicles": [0, 9],
                                            "object": 1, "alpha": 0}]}
    sec[0x64] = [{"tag": 1, "kinds": [0], "tris": [L.SwitchTri(*(t9 + [0x800]))]}]
    sec[0x68] = [{"owner": 7, "kind": 0, "areas": [L.GateArea(0, 0, 10, 0, 0, 10)], "word": 5,
                  "tris": [L.GateTri(*(t9 + [0, 3]))], "ids": [3]},
                 {"owner": 7, "kind": 1, "states": [0x0102], "tris": [], "ids": []}]
    sec[0x6C] = [[L.CollTri(*(t9 + [0x400, 1, 0]))], []]
    sec[0x70] = [[], [L.CollTri(*(t9 + [0, 0, 0]))]]
    sec[0x74] = {"nmtx": 0x40, "groups": [{"head": L.PartGroupHead(*([-1] + [0] * 9 + [1] + [0] * 9 + [1])),
                                           "items": [L.PartItem(*([0] * 9 + [0, 0] + [0x40, 1, 2, 3] * 3))]}]}
    # display lists: two lists; vis records point into them (block-relative)
    dl = struct.pack(">IIII", 0x04100000 | 0x20, 0x08000000, 0xBF000000, 0x0000000A) + struct.pack(">II", 0xB8000000, 0)
    dl += struct.pack(">II", 0x06000000, 0) + struct.pack(">II", 0xB8000000, 0)
    sec[0xA0] = [L.VisRecord(0, 0, 0x10, 1, 0)]
    sec[0xA4] = []
    sec[0xA8] = []
    sec[0xAC] = [L.VisRecord(0, 8, 0x10, 2, 1)]
    sec[0xB0] = [{"start": 0, "alt": 8, "end": 0x10, "id": 2, "cells": [0, 1]}]
    sec[0xB4] = []
    sec[0xB8] = []
    sec[0xBC] = []
    sec[0xC0] = None
    for s, v in sec.items():
        lv.sections[s] = L.Section(s, v)
    lv.dl = dl
    lv.dl_ptrs = {0x7C: 0x18, 0x80: 0x28, 0x88: 0, 0x8C: 0x18, 0x90: 0x18, 0x94: 0x18, 0x98: 0x18, 0x9C: 0x18}
    return lv


def synthetic_object():
    o = M.PackedObject()
    o.fields = {f[0]: 0 for f in M.OBJ_HEADER}
    o.fields.update(num_topple_keys=2, radius=50, toughness=3, people=1)
    o.vertices = [Vtx(i, i, i, 0, 0, 0, 1, 2, 3, 4) for i in range(4)]
    t9 = [1, 2, 3, 4, 5, 6, 7, 8, 9]
    vals = {0x1C: [M.Corner(1, 2, 3)] * 4, 0x20: [M.ObjBounds(-5, -5, 5, 5)], 0x24: [M.ObjTri(*(t9 + [1]))],
            0x28: [{"id": 1, "b5": 0, "b6": 0, "b7": 0, "w8": 0, "hC": 0, "hE": 0, "ids": [7]}],
            0x2C: [M.ObjPoint(1, 2, 3, 0)], 0x30: [M.Debris(*([0] * 11 + [3, 4] + [0] * 8))], 0x34: [],
            0x38: {"entries": [[1, 2, 3]], "groups": [{"words": [1, 2, 3, 4], "entries": [[5, 6, 7]]}]},
            0x3C: [1, 2], 0x40: [3], 0x44: [1, 2, 3], 0x48: [M.Tri25(*(t9 + [b"\0" * 7]))], 0x4C: None,
            0x10: struct.pack(">IIII", 0x04300040, 0x09000000, 0xB8000000, 0)}
    for s, v in vals.items():
        o.sections[s] = [v, b"", None]
    o.interior = {0x18: 8}
    return o


def synthetic_model():
    m = M.VehicleModel()
    vals = {0x00: {"lead": [], "runs": [{"h": [1, 2, 3], "words": [4, 5]}]},
            0x04: {"lead": [1, 0], "runs": [{"h": [1, 2, 3, 4, 5], "words": [6]}]},
            0x08: {"words": [1], "rows": [list(range(10))]}, 0x0C: None,
            0x10: {"table": [0, 1, 1], "table_tail": "", "blocks": [
                {"head": "01020300", "groups": [{"a": 1, "b": 2, "keys": [list(range(10))]}], "tail": ""},
                {"head": "", "groups": [], "tail": ""}]},
            0x14: [Vtx(1, 2, 3, 0, 0, 0, 9, 9, 9, 9)] * 3, 0x18: [0x40, 0x800, 0x10000, 0],
            0x1C: struct.pack(">IIII", 0x04200020, 0x06000000, 0xB8000000, 0)}
    for s, v in vals.items():
        m.sections[s] = [v, b"", None]
    m.interior = {s: 0 for s in M.VehicleModel.INTERIOR}
    return m


# ------------------------------------------------------------- synthetic

class TestGzip(unittest.TestCase):
    def test_member_roundtrip(self):
        data = bytes(range(256)) * 40
        m = gz.Member(data, "x.raw", 855417165)
        blob = m.build(gz.compress_zlib(data))
        r = gz.read_member(b"\0" + blob + b"tail", 0)        # one pad byte is skipped, as kiunzip does
        self.assertEqual(r.data, data)
        self.assertEqual(r.name, "x.raw")
        self.assertEqual(r.mtime, 855417165)
        self.assertEqual(r.pad, 1)
        self.assertEqual(r.end, 1 + len(blob))
        self.assertTrue(r.trailer_ok)
        self.assertEqual(r.header(), blob[:len(r.header())])

    def test_two_members(self):
        a, b = b"first" * 100, b"second" * 50
        blob = gz.Member(a, "a.raw").build(gz.compress_zlib(a)) + gz.Member(b, "a_dl.raw").build(gz.compress_zlib(b))
        ms = gz.read_members(blob, 0, 2)
        self.assertEqual([m.data for m in ms], [a, b])

    def test_bad_method(self):
        blob = bytearray(gz.Member(b"x", "n").build(gz.compress_zlib(b"x")))
        blob[2] = 7
        with self.assertRaises(Exception):
            gz.read_member(bytes(blob), 0)


class TestLzss(unittest.TestCase):
    def test_roundtrip(self):
        rnd = random.Random(1)
        samples = [b"", b"a", b"abcabcabcabcabc" * 20, bytes(rnd.randrange(4) for _ in range(5000)),
                   bytes(rnd.randrange(256) for _ in range(700)), b"\0" * 3000]
        for bits in (10, 12, 13):
            for s in samples:
                c = lzss.compress(s, bits)
                d, end = lzss.decompress(c, bits)
                self.assertEqual(d, s, "bits %d len %d" % (bits, len(s)))
                self.assertEqual(end, len(c))
                self.assertEqual(end % 2, 0)

    def test_hand_stream(self):
        # 1 'A', 1 'B', then a copy of 3+1 bytes from window position 1 (bits = 10), then the end
        bits = []

        def put(v, n):
            bits.extend((v >> (n - 1 - i)) & 1 for i in range(n))
        put(1, 1); put(0x41, 8); put(1, 1); put(0x42, 8)
        put(0, 1); put(1, 10); put(1, 6)
        put(0, 1); put(0, 10)
        bits += [0] * (-len(bits) % 16)
        data = bytes(int("".join(map(str, bits[i:i + 8])), 2) for i in range(0, len(bits), 8))
        out, end = lzss.decompress(data, 10)
        self.assertEqual(out, b"ABABAB")


class TestTexture(unittest.TestCase):
    def test_type1_literal_and_copy(self):
        toks = [0x7FFF, 0x0001, 0x8000 | (4 << 5) | 2]       # two texels, then copy 2 halfwords from 4 bytes back
        out = texture.decode(struct.pack(">3H", *toks), 1)
        self.assertEqual(struct.unpack(">4H", out), (0xFFBF, 0x0001, 0xFFBF, 0x0001))

    def test_type0_drops_tail(self):
        self.assertEqual(texture.decode(bytes(range(13)), 0), bytes(range(8)))

    def test_types(self):
        self.assertEqual(texture.decode(struct.pack(">H", 0x7FFF), 2), struct.pack(">I", 0xF0F0F0E0))
        self.assertEqual(texture.decode(struct.pack(">H", 0x7F01), 3), bytes((0xFE, 0x02)))
        self.assertEqual(texture.decode(struct.pack(">H", 0x3F3F), 6), bytes((0xEE, 0xEE)))
        pal = struct.pack(">128H", *range(128))
        self.assertEqual(texture.decode(struct.pack(">H", 0x0405), 4, pal), struct.pack(">HH", 4, 5))
        self.assertEqual(texture.decode(struct.pack(">H", 0x001F), 5, struct.pack(">2H", 0, 0x7FFF)),
                         struct.pack(">I", 0xF8F8F8F0))

    def test_rgba(self):
        rgba = texture.to_rgba8(struct.pack(">H", 0xF801), 0, 16, 1, 1)
        self.assertEqual(rgba, bytes((255, 0, 0, 255)))
        self.assertEqual(texture.to_rgba8(bytes((0xF0,)), 4, 4, 2, 1), bytes((255,) * 4 + (0,) * 4))


class TestPng(unittest.TestCase):
    def test_roundtrip(self):
        rgba = bytes(range(4 * 6 * 5)) * 1
        w, h, back = png.decode_rgba(png.encode_rgba(6, 5, rgba))
        self.assertEqual((w, h, back), (6, 5, rgba))


class TestCrc(unittest.TestCase):
    def test_fix(self):
        rom = bytearray(0x101000)
        for i in range(0x1000, 0x101000, 4):
            struct.pack_into(">I", rom, i, (i * 2654435761) & 0xFFFFFFFF)
        rombuild.fix_crc(rom)
        self.assertEqual(struct.unpack_from(">II", rom, 0x10), rombuild.cic6102_crc(rom))
        self.assertNotEqual(rombuild.cic6102_crc(rom), (0, 0))


class TestLevel(unittest.TestCase):
    def test_write_parse_write(self):
        lv = synthetic_level()
        data, split = lv.write()
        p = L.Level.parse(data, split)
        self.assertEqual(p.notes, [])
        self.assertEqual(p.write(), (data, split))
        self.assertEqual(json.loads(json.dumps(p.to_json())), json.loads(json.dumps(lv.to_json())))
        self.assertEqual(L.Level.from_json(json.loads(json.dumps(p.to_json()))).write(), (data, split))
        self.assertEqual(p["objects"][1].x, 100)
        self.assertEqual(p["coll_grid_b"][1][0].x0, 10)

    def test_grow_keeps_references(self):
        small = synthetic_level(3)
        big = synthetic_level(10)
        d1, s1 = small.write()
        d2, s2 = big.write()
        self.assertGreater(s2, s1)
        p = L.Level.parse(d2, s2)
        self.assertEqual(len(p["objects"]), 10)
        # visibility records stay relative to the display-list block, which moved
        self.assertEqual(p["vis_records_3"][0].alt, 8)
        hdr = struct.unpack_from(">I", d2, 0x78)[0]
        rec = struct.unpack_from(">I", d2, struct.unpack_from(">I", d2, 0xAC)[0] + 4)[0]
        self.assertEqual(rec, hdr + 8)
        self.assertEqual(hdr % 16, 0)
        # every later section moved by the same amount
        self.assertEqual(p.write(), (d2, s2))

    def test_odd_bytes_kept(self):
        # bytes the format doesn't explain after a section's records are kept as its tail
        lv = synthetic_level()
        lv.sections[0x54].tail = b"\x5A\x00\x00"
        lv.sections[0x20].tail = b"\x01\x02"
        data, split = lv.write()
        p = L.Level.parse(data, split)
        self.assertEqual(p.notes, [])
        self.assertTrue(p.sections[0x54].tail.startswith(b"\x5A"))
        self.assertEqual(p.sections[0x20].tail, b"\x01\x02")
        self.assertEqual(p.write(), (data, split))

    def test_opaque_fallback(self):
        p = L.Level.parse(b"\1" * 0x100)
        self.assertIsNotNone(p.raw)
        self.assertEqual(p.write(), (b"\1" * 0x100, 0x100))


class TestModels(unittest.TestCase):
    def test_object(self):
        o = synthetic_object()
        data, split = o.write()
        p = M.PackedObject.parse(data, split)
        self.assertEqual(p.notes, [])
        self.assertEqual(p.write(), (data, split))
        self.assertEqual(M.PackedObject.from_json(json.loads(json.dumps(p.to_json()))).write(), (data, split))
        self.assertEqual(split % 16, 0)

    def test_model(self):
        m = synthetic_model()
        data, split = m.write()
        p = M.VehicleModel.parse(data, split)
        self.assertEqual(p.notes, [])
        self.assertEqual(p.write(), (data, split))
        self.assertEqual(M.VehicleModel.from_json(json.loads(json.dumps(p.to_json()))).write(), (data, split))
        self.assertEqual(len(p.vertices()), 3)

    def test_gltf(self):
        from bcfiles import gltf
        o = synthetic_object()
        o.sections[0x10][0] = struct.pack(">IIII", 0x04300040, 0x09000000, 0xBF000000, 0x00000A14) + \
            struct.pack(">II", 0xB8000000, 0)
        with tempfile.TemporaryDirectory() as d:
            path, st = gltf.export_object(None, o, d, "t")
            with open(path) as fh:
                g = json.load(fh)
            self.assertEqual(st["triangles"], 1)
            self.assertEqual(g["accessors"][g["meshes"][0]["primitives"][0]["indices"]]["count"], 3)
            self.assertEqual(os.path.getsize(os.path.join(d, "t.bin")), g["buffers"][0]["byteLength"])


# ------------------------------------------------------------------ ROM

@unittest.skipUnless(ROM_PATH or "--rom" in sys.argv, "no --rom")
class TestRom(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        from bcfiles import Rom
        cls.rom = Rom.load(ROM_PATH)

    def test_roundtrip_every_file(self):
        from bcfiles.cli import roundtrip
        res, bad = roundtrip(self.rom)
        print("\n  %s: %s" % (self.rom.version, "; ".join("%s %s" % (k, " ".join("%s=%d" % kv for kv in r.items()))
                                                         for k, r in res.items())))
        self.assertEqual(bad, [])
        self.assertEqual(res["level"]["files"], 60)
        self.assertEqual(res["model"]["files"], 21)
        self.assertEqual(res["scene"]["files"], 3)
        for r in res.values():
            self.assertEqual(r["bytes_equal"], r["files"])
            self.assertEqual(r["typed"], r["files"])

    def test_lzss_files(self):
        n = 0
        for f in self.rom.files():
            if f.kind != "lzss":
                continue
            data, end = self.rom.lzss_file(f)
            self.assertGreater(len(data), 0)
            c = lzss.compress(data, f.info["bits"])
            self.assertEqual(lzss.decompress(c, f.info["bits"])[0], data, f.name)
            n += 1
        self.assertEqual(n, 6)

    def test_layout_walk(self):
        """the walk finds every file where the code's constants say it is"""
        from bcfiles import romfs
        for n, s in enumerate(romfs.LEVEL_ROM):
            self.assertEqual(self.rom.level(n).start, s)
            nxt = romfs.LEVELS_END if n == 59 else min(x for x in romfs.LEVEL_ROM if x > s)
            self.assertEqual(self.rom.level(n).info["slot_end"], nxt)
        for i, (s, e, types) in enumerate(romfs.MODEL_ROM):
            self.assertEqual((self.rom.model(i).start, self.rom.model(i).info["slot_end"]), (s, e))
            self.assertEqual(self.rom.model(i).info["types"], types)
        self.assertEqual(self.rom.obj_table_rom, romfs.OBJ_TABLE)
        self.assertEqual(self.rom.static_rom, romfs.STATIC_SEGMENT)
        self.assertEqual(self.rom.code_rom, romfs.HD_CODE)
        scenes = [f for f in self.rom.files() if f.kind == "scene"]
        self.assertEqual([(f.start, f.info["slot_end"]) for f in scenes][:2], romfs.SCENE_ROM[:2])
        lz = [(f.start, f.info["bits"]) for f in self.rom.files() if f.kind == "lzss"]
        self.assertEqual(lz, [(s, b) for s, e, b, w in romfs.LZSS_FILES])

    def test_texture_table(self):
        t = self.rom.tex_table
        self.assertEqual(len(t), 4096)
        n = 0
        for e in t:
            if e.type <= 6 and e.size and e.dma_len >= e.size:
                texture.decode(self.rom.texture_packed(e.index), e.type)
                n += 1
        self.assertGreater(n, 3000)

    def test_unchanged_build(self):
        from bcfiles.cli import PARSERS
        patches = []
        for f in [self.rom.level(n) for n in range(60)] + [x for x in self.rom.files() if x.kind in PARSERS and x.kind != "level"]:
            p = PARSERS[f.kind].parse(f.data, f.split)
            data, split = p.write()
            blob = rombuild.build_members(f, data, split)
            self.assertEqual(blob, self.rom.data[f.start:f.end], f.name)
            patches.append(rombuild.Patch(f, blob, data != f.data))
        image, rest = rombuild.apply_in_place(self.rom.data, patches)
        self.assertEqual(rest, [])
        self.assertEqual(image, self.rom.data)

    def test_edit_in_place(self):
        f = self.rom.level(1)
        lv = L.Level.parse(f.data, f.split)
        lv["objects"][0].x += 256
        data, split = lv.write()
        self.assertEqual(split, f.split)
        blob, how = rombuild.pack_to_fit(f, data, split)
        pt = rombuild.Patch(f, blob, True, how)
        print("\n  level 1, object 0 moved: %d compressed bytes (%s), slot %d" % (len(blob), how, pt.slot))
        self.assertTrue(pt.fits)
        image, rest = rombuild.apply_in_place(self.rom.data, [pt])
        from bcfiles import Rom
        r2 = Rom(image)
        lv2 = L.Level.parse(r2.level(1).data, r2.level(1).split)
        self.assertEqual(lv2["objects"][0].x, lv["objects"][0].x)
        self.assertEqual(struct.unpack_from(">II", image, 0x10), rombuild.cic6102_crc(image))


if __name__ == "__main__":
    if "--rom" in sys.argv:
        i = sys.argv.index("--rom")
        ROM_PATH = sys.argv[i + 1]
        del sys.argv[i:i + 2]
        TestRom.__unittest_skip__ = False
    unittest.main(verbosity=1)
