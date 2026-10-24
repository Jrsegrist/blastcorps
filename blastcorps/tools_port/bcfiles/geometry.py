"""Level geometry: the triangles, and everything the game derives from them.

A level file holds three triangle sets and the structures built from them
(docs/systems/levels.md, collision.md, rendering.md):

  render  the display-list block (0x78..0x84) and its segment-8 vertices: every
          G_TRI1, with the state commands around it.  The block is a row of
          pieces (the 0x88 prefix, the packed lists 0x7C..0x80, the visibility
          records and groups 0xA0..0xBC, the sub-lists the fixed lists call,
          the fixed lists 0x90..0x9C); a piece's triangles are its own.
  ground  the floor triangles of the ground grid (0x30): nine s16, a surface
          class, a stop flag.  Every one is also a render triangle (same
          corners) on all 60 US levels.
  walls   the wall triangles of collision grids A and B (0x6C, 0x70): nine
          s16, heading, side byte, fail byte.  Mostly not drawn.

`Geometry.from_level` takes the three sets out of a parsed `level.Level`;
`Geometry.write_into` puts them back and regenerates what the game derives:

  ground grid  a triangle goes into every ground cell its x-z footprint meets
               (closed cell rectangles), except cells lying strictly inside it
               (tri_cells); cells list their triangles in one global order (a
               topological order of Rare's per-cell lists, so an unchanged
               level keeps Rare's order).
  wall grids   the same with the cell rectangles grown by WALL_MARGIN (50) map
               units on every side, half-open (touching the grown rectangle's
               high edge doesn't count): the game looks up one cell (the
               sphere centre's) and Rare listed walls near a cell edge in the
               neighbouring cells too.
  vis grid     per visibility cell: (min(0, lowest y), max(0, highest y)) over
               the drawn vertices inside the cell's closed rectangle.
  bounds       the ground triangles' x-z extent grown by BOUNDS_MARGIN (50).
  display      each piece is written again: its state commands in order, its
  lists        triangles re-packed into G_VTX loads of at most 16 vertices
               (F3D's vertex buffer), loaded between the same vertex-state
               commands as before (matrix, texture scale, lighting and
               texture-generation modes, G_DL); the vertices are laid out in
               load order.  Pieces whose triangles use vertices across such a
               command, or that move or cull geometry (G_MTX, G_POPMTX,
               G_CULLDL: the bridges' packed lists) keep their own command
               sequence ("frozen"); their vertices still move with an edit.
               All offsets into the block (header 0x7C..0x9C, the visibility
               records and groups, G_DL targets) follow.

Not derived here (left as the file has them): the guard triangles (0x24),
camera and chopper height zones (0x40, 0x44), switch and gate triangles
(0x64, 0x68), the moving-part groups (0x74), and every per-level table in
the code.
"""
import collections
import heapq
import struct

from .binio import FormatError, Vtx
from . import level as L

WALL_MARGIN = 50
WALL_HALF_OPEN = True
BOUNDS_MARGIN = 50
VTX_CACHE = 16

# commands that change how a vertex is transformed when it is loaded (F3D):
# the vertices loaded before one of these must not be used after it if the
# list is re-packed (their state is the old one)
VSTATE_OPS = {0x01, 0xBD, 0x03, 0xBC, 0xBB, 0x06, 0xBE}
G_FOG, G_LIGHTING, G_TEXTURE_GEN, G_TEXTURE_GEN_LINEAR = 0x10000, 0x20000, 0x40000, 0x80000
VSTATE_GEOM = G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR
FREEZE_OPS = {0xBE}   # G_CULLDL tests vertex-buffer slots: keep the piece as it is


def is_vstate(w0, w1):
    op = w0 >> 24
    if op in VSTATE_OPS:
        return True
    if op in (0xB6, 0xB7) and (w1 & VSTATE_GEOM):
        return True
    return False


class GeometryError(Exception):
    pass


# ------------------------------------------------------------------ 2-D triangle / rectangle test

def tri_meets_rect(v9, rx0, rz0, rx1, rz1):
    """does the x-z footprint of the triangle (nine coords) meet the closed rectangle?"""
    P = ((v9[0], v9[2]), (v9[3], v9[5]), (v9[6], v9[8]))
    xs = (P[0][0], P[1][0], P[2][0])
    zs = (P[0][1], P[1][1], P[2][1])
    if max(xs) < rx0 or min(xs) > rx1 or max(zs) < rz0 or min(zs) > rz1:
        return False
    R = ((rx0, rz0), (rx1, rz0), (rx1, rz1), (rx0, rz1))
    flat = True
    for i in range(3):
        a, b, c = P[i], P[(i + 1) % 3], P[(i + 2) % 3]
        nx, nz = a[1] - b[1], b[0] - a[0]
        sc = nx * (c[0] - a[0]) + nz * (c[1] - a[1])
        if sc == 0:
            continue
        flat = False
        if sc > 0:
            if all(nx * (r[0] - a[0]) + nz * (r[1] - a[1]) < 0 for r in R):
                return False
        else:
            if all(nx * (r[0] - a[0]) + nz * (r[1] - a[1]) > 0 for r in R):
                return False
    if flat:
        # the corners on one line (a segment, or a point): the rectangle must
        # reach the line
        a, b = max(((p, q) for p in P for q in P), key=lambda pq: (pq[0][0] - pq[1][0]) ** 2 + (pq[0][1] - pq[1][1]) ** 2)
        nx, nz = a[1] - b[1], b[0] - a[0]
        if nx or nz:
            s = [nx * (r[0] - a[0]) + nz * (r[1] - a[1]) for r in R]
            if min(s) > 0 or max(s) < 0:
                return False
    return True


def _covers(v9, x0, z0, x1, z1):
    """is the rectangle strictly inside the triangle's footprint (all four
    corners off its edges, so no corner or edge of the triangle reaches it)?"""
    P = ((v9[0], v9[2]), (v9[3], v9[5]), (v9[6], v9[8]))
    for (px, pz) in ((x0, z0), (x1, z0), (x1, z1), (x0, z1)):
        s = [(P[(i + 1) % 3][0] - P[i][0]) * (pz - P[i][1]) - (P[(i + 1) % 3][1] - P[i][1]) * (px - P[i][0])
             for i in range(3)]
        if not (all(x > 0 for x in s) or all(x < 0 for x in s)):
            return False
    return True


def tri_cells(v9, cw, cd, cols, rows, margin=0, half_open=False):
    """the cells (row-major indices) whose rectangle, grown by margin, the
    triangle meets, as Rare's tool chose them (docs/systems/levels.md):
      - the closed rectangle (half_open: a cell is a candidate only from the
        floor of the triangle's grown low extent to the floor of its high
        extent, so touching a cell's high edge doesn't count);
      - not a cell strictly inside the triangle (no corner of the triangle in
        it, no edge through or along it): Rare's grids leave those out, so a
        large triangle is listed only in the cells its edges and corners reach
        (L28 and L48 each have such a cell, which the game then answers from
        the other triangles listed there)"""
    xs = (v9[0], v9[3], v9[6])
    zs = (v9[2], v9[5], v9[8])
    lox = 0 if half_open in (True, "x") else 1
    loz = 0 if half_open in (True, "z") else 1
    cx0 = max((min(xs) - margin) // cw - lox, 0)
    cx1 = min((max(xs) + margin) // cw + lox, cols - 1)
    cz0 = max((min(zs) - margin) // cd - loz, 0)
    cz1 = min((max(zs) + margin) // cd + loz, rows - 1)
    out = []
    for cz in range(cz0, cz1 + 1):
        for cx in range(cx0, cx1 + 1):
            r = (cx * cw - margin, cz * cd - margin, (cx + 1) * cw + margin, (cz + 1) * cd + margin)
            if tri_meets_rect(v9, *r) and not _covers(v9, *r):
                out.append(cz * cols + cx)
    return out


# ------------------------------------------------------------------ grids: triangles in one global order

def grid_order(cells):
    """Rare's per-cell lists -> one list of triangles (value tuples) in a global
    order that keeps every cell's order.  A triangle listed k times in one cell
    (identical records) is k instances."""
    key_cells = []
    first = {}
    succ = collections.defaultdict(set)
    indeg = collections.Counter()
    for cell in cells:
        seen = collections.Counter()
        keys = []
        for t in cell:
            v = tuple(t._v)
            k = (v, seen[v])
            seen[v] += 1
            keys.append(k)
            if k not in first:
                first[k] = len(first)
        for a, b in zip(keys, keys[1:]):
            if b not in succ[a]:
                succ[a].add(b)
                indeg[b] += 1
        key_cells.append(keys)
    heap = [(first[k], k) for k in first if indeg[k] == 0]
    heapq.heapify(heap)
    order = []
    while heap:
        _, k = heapq.heappop(heap)
        order.append(k)
        for m in succ[k]:
            indeg[m] -= 1
            if indeg[m] == 0:
                heapq.heappush(heap, (first[m], m))
    if len(order) != len(first):
        # no order keeps every cell's (not on the US levels): first appearance
        order = sorted(first, key=first.get)
    return [list(k[0]) for k in order]


def build_grid(tris, cols, rows, cw, cd, margin, cls, half_open=False):
    cells = [[] for _ in range(cols * rows)]
    for t in tris:
        for c in tri_cells(t[:9], cw, cd, cols, rows, margin, half_open):
            cells[c].append(cls(*t))
    return cells


# ------------------------------------------------------------------ the display-list block

def _vtx_tuple(v):
    return tuple(v._v)


class Piece:
    """a run of commands of the display-list block.  kind: prefix, packed, rec,
    grp, sub (a list some G_DL calls), fixed (0x90..0x9C), raw (bytes nothing
    points at).  ops:
      ('c', w0, w1)              a command kept as it is
      ('dl', w0, target)         G_DL; target = the original offset of the list it calls
      ('t', w0, flag, (v, v, v)) a triangle (v: Vtx value tuples)             [re-packed pieces]
      ('vtx', w0, [v, ...])      a G_VTX with its vertices                    [frozen pieces]
      ('tri', w0, w1, (v, v, v)) a G_TRI1 as it is, with its vertices' values [frozen pieces]
    """
    __slots__ = ("kind", "start", "end", "ops", "frozen", "why", "setup", "setup0", "ref", "orig")

    def __init__(self, kind, start, end, ref=None):
        self.kind, self.start, self.end, self.ref = kind, start, end, ref
        self.ops = []
        self.orig = []
        self.setup0 = 0
        self.frozen = False
        self.why = ""
        self.setup = 0      # rec/grp: commands before the alternate start

    def triangles(self):
        for op in self.ops:
            if op[0] == 't':
                yield op[3]
            elif op[0] == 'tri':
                yield op[3]


def _list_end(dl, q):
    while q + 8 <= len(dl):
        w0 = struct.unpack_from(">I", dl, q)[0]
        q += 8
        if w0 >> 24 == 0xB8:
            return q
    raise GeometryError("display list at 0x%X has no G_ENDDL" % q)


def split_pieces(lv):
    dl = lv.dl
    p = lv.dl_ptrs
    pieces = {}

    def add(kind, a, b, ref=None):
        if a == b:
            return
        if a in pieces:
            if pieces[a].end != b:
                raise GeometryError("two lists start at 0x%X with different ends" % a)
            return
        pieces[a] = Piece(kind, a, b, ref)
    add("prefix", p[0x88], p[0x8C])
    add("packed", p[0x7C], p[0x80])
    for k in range(4):
        for i, r in enumerate(lv.sections[0xA0 + 4 * k].value):
            add("rec", r.start, r.end, (k, i))
            pieces[r.start].setup = (r.alt - r.start) // 8
        for i, g in enumerate(lv.sections[0xB0 + 4 * k].value):
            add("grp", g["start"], g["end"], (k, i))
            pieces[g["start"]].setup = (g["alt"] - g["start"]) // 8
    for s in (0x90, 0x94, 0x98, 0x9C):
        add("fixed", p[s], _list_end(dl, p[s]), s)
    targets = set()
    for q in range(0, len(dl) - 7, 8):
        w0, w1 = struct.unpack_from(">II", dl, q)
        if w0 >> 24 == 0x06:
            if w1 >= len(dl) or w1 % 8:
                raise GeometryError("G_DL at 0x%X calls 0x%X, outside the block" % (q, w1))
            targets.add(w1)
    for t in targets:
        if t not in pieces:
            add("sub", t, _list_end(dl, t))
    out = []
    pos = 0
    for a in sorted(pieces):
        pc = pieces[a]
        if a < pos:
            raise GeometryError("lists overlap at 0x%X" % a)
        if a > pos:
            out.append(Piece("raw", pos, a))
        out.append(pc)
        pos = pc.end
    if pos < len(dl):
        out.append(Piece("raw", pos, len(dl)))
    return out


def _aabb(vs):
    if not vs:
        return None
    return (min(v[0] for v in vs), max(v[0] for v in vs), min(v[1] for v in vs), max(v[1] for v in vs),
            min(v[2] for v in vs), max(v[2] for v in vs))


def _box_pattern(v, bb):
    """which of min / max each coordinate of a box corner is"""
    return tuple(0 if v[k] == bb[2 * k] else 1 for k in range(3))


def _box_from(pattern, rest, bb):
    return [(bb[p[0]], bb[2 + p[1]], bb[4 + p[2]]) + tuple(r) for p, r in zip(pattern, rest)]


def decode_piece(pc, dl, verts):
    """fill pc.ops; decide whether the piece is frozen"""
    cmds = [struct.unpack_from(">II", dl, q) for q in range(pc.start, pc.end, 8)]
    if (pc.end - pc.start) % 8:
        raise GeometryError("piece 0x%X..0x%X is not whole commands" % (pc.start, pc.end))
    if pc.kind == "raw":
        pc.ops = [('c', w0, w1) for w0, w1 in cmds]
        pc.orig = list(pc.ops)
        pc.frozen = True
        pc.why = "raw"
        return
    pc.setup0 = pc.setup
    nverts = len(verts)
    slot = [None] * VTX_CACHE        # (value, epoch)
    epoch = 0
    ops = []
    why = []
    for i, (w0, w1) in enumerate(cmds):
        op = w0 >> 24
        if op == 0x04:
            n = ((w0 >> 20) & 0xF) + 1
            v0 = (w0 >> 16) & 0xF
            seg, off = w1 >> 24, w1 & 0xFFFFFF
            if seg != 8 or off % 16 or off // 16 + n > nverts or v0 + n > VTX_CACHE:
                raise GeometryError("G_VTX %08X %08X at 0x%X: not segment-8 vertices of the file" % (w0, w1, pc.start + 8 * i))
            vs = [_vtx_tuple(verts[off // 16 + j]) for j in range(n)]
            for j in range(n):
                slot[v0 + j] = (vs[j], epoch)
            ops.append(('vtx', w0, vs, off // 16))
            if i < pc.setup:
                why.append("a vertex load before the alternate start")
        elif op == 0xBF:
            idx = [((w1 >> s) & 0xFF) // 10 for s in (16, 8, 0)]
            if any(((w1 >> s) & 0xFF) % 10 for s in (16, 8, 0)) or any(x >= VTX_CACHE or slot[x] is None for x in idx):
                raise GeometryError("G_TRI1 %08X at 0x%X uses a vertex never loaded" % (w1, pc.start + 8 * i))
            vals = tuple(slot[x][0] for x in idx)
            if any(slot[x][1] != epoch for x in idx):
                why.append("a triangle uses vertices loaded before a vertex-state command")
            ops.append(('tri', w0, w1, vals))
        elif op == 0x06:
            ops.append(('dl', w0, w1))
            epoch += 1
        else:
            ops.append(('c', w0, w1))
            if op in FREEZE_OPS and not (i == 1 and ops[0][0] == 'vtx' and pc.setup == 0):
                why.append("G_CULLDL")
            if is_vstate(w0, w1):
                epoch += 1
    if pc.kind == "packed":
        why.append("packed list")
    cull = None
    if len(ops) >= 2 and ops[0][0] == 'vtx' and ops[1][0] == 'c' and ops[1][1] >> 24 == 0xBE and pc.setup == 0:
        # the sub-lists' prologue: a box of 8 vertices and G_CULLDL over it; the
        # box is the bounding box of the list's vertices (1013 of the 1018 US
        # lists; the other 5 are the bridges' packed lists, which stay frozen)
        box = ops[0][2]
        body = [v for o in ops[2:] if o[0] == 'tri' for v in o[3]]
        bb = _aabb(body)
        if bb is not None and _aabb(box) == bb:
            cull = ('cull', ops[0][1], [_box_pattern(v, bb) for v in box], [v[3:] for v in box], ops[1][1], ops[1][2], box)
        else:
            why.append("G_CULLDL box is not the list's bounding box")
    pc.frozen = bool(why)
    pc.why = why[0] if why else ""
    pc.orig = list(ops)               # the commands as the file has them (keep-Rare's-layout mode)
    if not pc.frozen:
        # re-packable: drop the loads, keep the triangles as values
        new = [cull] if cull else []
        for o in (ops[2:] if cull else ops):
            if o[0] == 'vtx':
                continue
            if o[0] == 'tri':
                new.append(('t', o[1], (o[2] >> 24) & 0xFF, o[3]))
            else:
                new.append(o)
        # the alternate start counts commands without the loads
        if pc.setup:
            pc.setup = sum(1 for o in ops[:pc.setup] if o[0] != 'vtx')
        ops = new
    pc.ops = ops


class _VertexArray:
    """the segment-8 vertices being laid out; `base`: start from the file's
    own array (keep-Rare's-layout mode)"""

    def __init__(self, base=None):
        self.verts = list(base or [])
        self.index = {}

    def add_at(self, vs, off):
        """vs where the file had it, if that is still there"""
        if off is not None and self.verts[off:off + len(vs)] == list(vs):
            return off
        return self.add(vs)

    def add(self, vs):
        key = tuple(vs)
        off = self.index.get(key)
        if off is None:
            off = len(self.verts)
            self.verts.extend(vs)
            self.index[key] = off
        return off


def _emit_tri(out, w0, flag, idx):
    out.append((w0, (flag << 24) | (idx[0] * 10 << 16) | (idx[1] * 10 << 8) | idx[2] * 10))


def _emit_batch(out, va, ops, i, loaded):
    """a G_VTX for the triangles from ops[i] on, as many as fit"""
    batch = []
    j = i
    while j < len(ops):
        o = ops[j]
        if o[0] == 't':
            need = []
            for v in o[3]:
                if v not in batch and v not in need:
                    need.append(v)
            if len(batch) + len(need) > VTX_CACHE:
                break
            batch += need
        elif o[0] in ('dl', 'vtx', 'tri', 'cull') or (o[0] == 'c' and is_vstate(o[1], o[2])):
            break
        j += 1
    off = va.add(batch)
    n = len(batch)
    out.append((0x04000000 | ((n - 1) << 20) | (0 << 16) | (n * 16), 0x08000000 | (off * 16)))
    loaded.clear()
    for k, v in enumerate(batch):
        loaded[v] = k


def emit_piece(pc, va, keep=False):
    """-> ([(w0, w1)], setup command count, [(index in the output, G_DL target)]);
    keep: the piece's own commands and vertex loads, as the file had them"""
    out = []
    dls = []
    setup_out = None
    loaded = {}
    ops = pc.orig if keep else pc.ops
    setup = pc.setup0 if keep else pc.setup
    for i, o in enumerate(ops):
        if i == setup and pc.kind in ("rec", "grp"):
            setup_out = len(out)
        kind = o[0]
        if kind == 'cull':
            # the box: the bounding box of the list's vertices now
            bb = _aabb([v for x in ops if x[0] in ('t', 'tri') for v in x[3]])
            box = _box_from(o[2], o[3], bb) if bb is not None else o[6]
            off = va.add(box)
            out.append((o[1], 0x08000000 | (off * 16)))
            out.append((o[4], o[5]))
            loaded.clear()
        elif kind == 'c':
            out.append((o[1], o[2]))
            if is_vstate(o[1], o[2]):
                loaded.clear()
        elif kind == 'dl':
            dls.append((len(out), o[2]))
            out.append((o[1], 0))
            loaded.clear()
        elif kind == 'vtx':
            off = va.add_at(o[2], o[3]) if keep else va.add(o[2])
            out.append((o[1], 0x08000000 | (off * 16)))
            loaded.clear()       # (a frozen piece's own loads: an added triangle after them loads its own)
        elif kind == 'tri':
            out.append((o[1], o[2]))
        elif kind == 't':
            if not all(v in loaded for v in o[3]):
                _emit_batch(out, va, ops, i, loaded)
            _emit_tri(out, o[1], o[2], [loaded[v] for v in o[3]])
    if pc.kind in ("rec", "grp") and setup_out is None:
        setup_out = len(out) if setup >= len(ops) else 0
    return out, setup_out, dls


# ------------------------------------------------------------------ the model

class Geometry:
    """the triangles of a level (see the module comment).

    render: [Piece]; ground: [[x0, y0, z0, x1, y1, z1, x2, y2, z2, surface, flag]];
    walls: {0x6C: [[nine coords, heading, side, b15]], 0x70: [...]}"""

    def __init__(self):
        self.pieces = []
        self.ground = []
        self.walls = {0x6C: [], 0x70: []}
        self.notes = []
        self.orig_verts = []

    # ---------------------------------------------------------- from a level
    @classmethod
    def from_level(cls, lv):
        if lv.raw is not None:
            raise GeometryError("the level file doesn't parse")
        for s in (0x30, 0x6C, 0x70, 0x48, 0x4C) + tuple(range(0xA0, 0xC0, 4)):
            if lv.sections[s].raw is not None:
                raise GeometryError("section %s is kept raw" % L.SECTION_NAMES[s])
        g = cls()
        g.orig_verts = [_vtx_tuple(v) for v in lv.vertices]
        g.pieces = split_pieces(lv)
        for pc in g.pieces:
            decode_piece(pc, lv.dl, lv.vertices)
        g.ground = grid_order(lv["ground_grid"])
        g.walls = {0x6C: grid_order(lv["coll_grid_a"]), 0x70: grid_order(lv["coll_grid_b"])}
        return g

    # ---------------------------------------------------------- triangle access
    def render_tris(self):
        """[(piece index, op index, (v, v, v))] in block order: the render triangle ids"""
        out = []
        for pi, pc in enumerate(self.pieces):
            for oi, o in enumerate(pc.ops):
                if o[0] in ('t', 'tri'):
                    out.append((pi, oi, o[3]))
        return out

    def stats(self):
        fr = collections.Counter(pc.why for pc in self.pieces if pc.frozen and pc.kind != "raw")
        return {"pieces": len(self.pieces), "frozen": sum(1 for pc in self.pieces if pc.frozen and pc.kind != "raw"),
                "frozen_why": dict(fr), "render_tris": len(self.render_tris()), "ground_tris": len(self.ground),
                "walls_a": len(self.walls[0x6C]), "walls_b": len(self.walls[0x70])}

    # ---------------------------------------------------------- derived structures
    def vis_heights(self, h):
        cw, cd, cols, rows = h["vis_cell_w"], h["vis_cell_d"], h["vis_cols"], h["vis_rows"]
        mn = [0] * (cols * rows)
        mx = [0] * (cols * rows)
        seen = set()
        for pc in self.pieces:
            for tri in pc.triangles():
                for v in tri:
                    p = (v[0], v[1], v[2])
                    if p in seen:
                        continue
                    seen.add(p)
                    x, y, z = p
                    xs = [x // cw] + ([x // cw - 1] if x % cw == 0 else [])
                    zs = [z // cd] + ([z // cd - 1] if z % cd == 0 else [])
                    for cx in xs:
                        if not 0 <= cx < cols:
                            continue
                        for cz in zs:
                            if not 0 <= cz < rows:
                                continue
                            c = cz * cols + cx
                            if y < mn[c]:
                                mn[c] = y
                            if y > mx[c]:
                                mx[c] = y
        return [[a, b] for a, b in zip(mn, mx)]

    def bounds(self):
        if not self.ground:
            return None
        xs = [t[k] for t in self.ground for k in (0, 3, 6)]
        zs = [t[k] for t in self.ground for k in (2, 5, 8)]
        return L.Bounds(min(xs) - BOUNDS_MARGIN, min(zs) - BOUNDS_MARGIN, max(xs) + BOUNDS_MARGIN, max(zs) + BOUNDS_MARGIN)

    def ground_grid(self, h):
        return build_grid(self.ground, h["ground_cols"], h["ground_rows"], h["ground_cell_w"], h["ground_cell_d"], 0,
                          L.GroundTri)

    def wall_grid(self, h, slot):
        return build_grid(self.walls[slot], h["coll_cols"], h["coll_rows"], h["coll_cell_w"], h["coll_cell_d"],
                          WALL_MARGIN, L.CollTri, WALL_HALF_OPEN)

    def build_dl(self, lv, base=None):
        """-> (block bytes, vertices, {orig offset: new offset}, piece outputs);
        base: the original Geometry: pieces it has unchanged keep their commands"""
        same = [False] * len(self.pieces)
        if base is not None and len(base.pieces) == len(self.pieces):
            same = [a.start == b.start and a.ops == b.ops for a, b in zip(self.pieces, base.pieces)]
        va = _VertexArray(base.orig_verts if base is not None else None)
        out = bytearray()
        bound = {}
        fix = []
        info = []
        for pc, keep in zip(self.pieces, same):
            cmds, setup, dls = emit_piece(pc, va, keep)
            base = len(out)
            bound.setdefault(pc.start, base)
            for (k, tgt) in dls:
                fix.append((base + 8 * k, tgt))
            for w0, w1 in cmds:
                out += struct.pack(">II", w0, w1)
            info.append((base, len(out), setup))
            bound[pc.end] = len(out)
        starts = {pc.start: inf[0] for pc, inf in zip(self.pieces, info)}
        for pos, tgt in fix:
            if tgt not in starts:
                raise GeometryError("G_DL to 0x%X: no list starts there" % tgt)
            struct.pack_into(">I", out, pos + 4, starts[tgt])
        return bytes(out), [Vtx(*v) for v in va.verts], bound, info

    # ---------------------------------------------------------- into a level
    def write_into(self, lv, parts=("ground", "walls", "vis", "bounds", "dl"), base=None):
        """regenerate the derived sections of lv (a level.Level) from this
        geometry.  base: the original level (a level.Level); then whatever the
        geometry's changes don't reach keeps Rare's data: a grid cell whose
        regenerated list is the same for both geometries keeps the file's list,
        likewise visibility cells and the bounds, and an unchanged display-list
        piece keeps its own commands and vertex loads (an unchanged geometry
        gives the original file back, byte for byte)"""
        h = lv.header
        bg = Geometry.from_level(base) if base is not None else None

        def keep_cells(new, old_gen, old_file):
            return [o if n_ == g_ else n_ for n_, g_, o in zip(new, old_gen, old_file)]

        def cells_key(cells):
            return [[t._v for t in c] for c in cells]
        if "dl" in parts:
            dl, verts, bound, info = self.build_dl(lv, bg)
            for s, v in list(lv.dl_ptrs.items()):
                if v not in bound:
                    raise GeometryError("header word 0x%X points into a list (0x%X)" % (s, v))
                lv.dl_ptrs[s] = bound[v]
            for pc, (a, b, setup) in zip(self.pieces, info):
                if pc.kind == "rec":
                    k, i = pc.ref
                    r = lv.sections[0xA0 + 4 * k].value[i]
                    r.start, r.alt, r.end = a, a + 8 * setup, b
                elif pc.kind == "grp":
                    k, i = pc.ref
                    g = lv.sections[0xB0 + 4 * k].value[i]
                    g["start"], g["alt"], g["end"] = a, a + 8 * setup, b
            lv.dl = dl
            lv.dl_trailer = None
            lv.vertices = verts
            lv.vtx_tail = b""
        if "ground" in parts:
            new = self.ground_grid(h)
            if bg is not None:
                old = bg.ground_grid(h)
                nk, ok = cells_key(new), cells_key(old)
                new = [base.sections[0x30].value[i] if nk[i] == ok[i] else new[i] for i in range(len(new))]
            lv.sections[0x30].value = new
            lv.sections[0x30].tail = b"" if bg is None else base.sections[0x30].tail
        if "walls" in parts:
            for s in (0x6C, 0x70):
                new = self.wall_grid(h, s)
                if bg is not None:
                    old = bg.wall_grid(h, s)
                    nk, ok = cells_key(new), cells_key(old)
                    new = [base.sections[s].value[i] if nk[i] == ok[i] else new[i] for i in range(len(new))]
                lv.sections[s].value = new
                lv.sections[s].tail = b"" if bg is None else base.sections[s].tail
        if "vis" in parts:
            new = self.vis_heights(h)
            if bg is not None:
                new = keep_cells(new, bg.vis_heights(h), [list(c) for c in base.sections[0x48].value])
            lv.sections[0x48].value = new
        if "bounds" in parts:
            b = self.bounds()
            if bg is not None and b == bg.bounds():
                b = base.sections[0x4C].value
            if b is not None:
                lv.sections[0x4C].value = b
        return lv


# ------------------------------------------------------------------ what the RSP draws (for comparisons)

def draw_stream(lv, start, end=None):
    """the state commands and triangles (resolved vertex values, the vertex
    state they were loaded under) the RSP meets running from `start` (G_DL
    followed): two blocks with equal streams draw the same pixels"""
    dl = lv.dl
    verts = lv.vertices
    slot = [None] * VTX_CACHE
    vstate = []
    stack = []
    pc = start
    out = []
    steps = 0
    while pc + 8 <= len(dl) and steps < 400000:
        if end is not None and pc >= end and not stack:
            break
        steps += 1
        w0, w1 = struct.unpack_from(">II", dl, pc)
        pc += 8
        op = w0 >> 24
        if op == 0x04:
            n = ((w0 >> 20) & 0xF) + 1
            v0 = (w0 >> 16) & 0xF
            off = (w1 & 0xFFFFFF) // 16
            st = tuple(vstate)
            for j in range(n):
                slot[v0 + j] = (_vtx_tuple(verts[off + j]), st) if off + j < len(verts) else None
        elif op == 0xBF:
            out.append(("T", w1 >> 24, tuple(slot[((w1 >> s) & 0xFF) // 10] for s in (16, 8, 0))))
        elif op == 0x06:
            out.append(("DL",))
            if not (w0 >> 16) & 1:
                stack.append((pc, end))
                end = None
            pc = w1
        elif op == 0xB8:
            if not stack:
                break
            pc, end = stack.pop()
        else:
            out.append(("C", w0, w1))
            if is_vstate(w0, w1):
                vstate.append((w0, w1))
    return out


def level_draw_streams(lv):
    """every entry point the game draws from: the fixed lists, the packed lists,
    each visibility record (both starts) and group"""
    p = lv.dl_ptrs
    s = {}
    for k in (0x90, 0x94, 0x98, 0x9C):
        s["fixed%X" % k] = draw_stream(lv, p[k])
    s["prefix"] = draw_stream(lv, p[0x88], p[0x8C])
    # the packed lists: each to its G_ENDDL
    q = p[0x7C]
    i = 0
    while q < p[0x80]:
        e = _list_end(lv.dl, q)
        s["packed%d" % i] = draw_stream(lv, q, e)
        q = e
        i += 1
    for k in range(4):
        for i, r in enumerate(lv.sections[0xA0 + 4 * k].value):
            s["rec%d.%d" % (k, i)] = draw_stream(lv, r.start, r.end)
            s["rec%d.%d.alt" % (k, i)] = draw_stream(lv, r.alt, r.end)
        for i, g in enumerate(lv.sections[0xB0 + 4 * k].value):
            s["grp%d.%d" % (k, i)] = draw_stream(lv, g["start"], g["end"])
            s["grp%d.%d.alt" % (k, i)] = draw_stream(lv, g["alt"], g["end"])
    return s
