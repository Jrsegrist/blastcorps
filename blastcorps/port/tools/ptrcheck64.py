#!/usr/bin/env python3
"""Pointer-width audit of the game C for the 64-bit build (port/README.md, "64-bit").

The 64-bit exe keeps N64 memory at its N64 addresses with N64 layouts: every
pointer stored in N64 memory (a struct field, a pinned global, an element of
a pinned table) must stay 4 bytes (N64PTR(T), include/game/n64ptr.h).  This
reads clang's AST of each translation unit (x86_64 target, the 64-bit build's
flags) and reports what still has a host-width (8-byte) pointer:

  FIELD  record.field   a pointer field of a record (struct/union)
  FNPTR  record.field   a function pointer field (clang can't load __ptr32
                        function pointers correctly: use N64FNPTR, a u32)
  VAR    name           a pinned global (in ADDRS, or named D_<addr>) holding a
                        pointer
  CAST   file:line      a cast to a pointer to a host-width pointer
                        (`*(u8 **) p` reads 8 bytes of N64 memory)

usage: ptrcheck64.py ADDRS ALLOW OUT -- CLANG_CMD... -- FILE.c...
  ALLOW  lines 'FIELD record.field' / 'VAR name' / 'CAST file:line' that are
         known host-only (a reason after '#'); 'FIELD record.*' for a whole
         record
  Exit status 1 if anything not allowed is found.
"""
import fnmatch
import re
import subprocess
import sys

ANY = re.compile(r"^([ |`-]*)\w+ 0x[0-9a-f]+ ")
# a signed integer of at most 32 bits: sign-extends when cast to a 64-bit pointer
SIGNED32 = {"int", "short", "signed char", "char", "long"}
DECL = re.compile(r"^(?P<indent>[ |`-]*)(?P<kind>RecordDecl|FieldDecl|VarDecl|CStyleCastExpr|TypedefDecl)"
                  r" 0x[0-9a-f]+ (?P<rest>.*)$")
TYPE = re.compile(r"'([^']*)'(?::'([^']*)')?")
LOCTOK = re.compile(r"(?:([^\s<>,()]+\.[ch]):(\d+):\d+|line:(\d+):\d+)")


def first_type(rest):
    ms = TYPE.findall(rest)
    if not ms:
        return None
    a, b = ms[0]
    return b or a


def host_ptr(t):
    """does type string t (canonical) contain a pointer that is not __ptr32?
    (clang prints an address-space pointer's qualifiers in front of the
    pointee, `__uptr __ptr32 unsigned char *`, so count them)"""
    if t is None:
        return False
    return t.count("__ptr32") < t.count("*")


def is_fnptr(t):
    return t is not None and re.search(r"\(\s*\*", t) is not None


def ptr_to_host_ptr(t):
    """T ** or T *const * etc.: a pointer whose pointee is a host-width pointer"""
    if t is None or is_fnptr(t):
        return False
    # every pointer level but the outermost must be __ptr32
    return t.count("*") >= 2 and t.count("__ptr32") < t.count("*") - 1


def main():
    a = sys.argv[1:]
    addrs_path, allow_path, out_path = a[0], a[1], a[2]
    i = a.index("--", 3)
    j = a.index("--", i + 1)
    cmd, files = a[i + 1:j], a[j + 1:]
    pinned = set()
    for line in open(addrs_path):
        p = line.split()
        if p:
            pinned.add(p[0])
    allow = set()
    try:
        for line in open(allow_path):
            line = line.split("#")[0].strip()
            if line:
                allow.add(line)
    except FileNotFoundError:
        pass

    def allowed(k):
        return any(fnmatch.fnmatchcase(k, a) for a in allow)

    found = {}
    for f in files:
        r = subprocess.run(cmd + ["-fsyntax-only", "-Xclang", "-ast-dump", "-fno-color-diagnostics", f],
                           capture_output=True, text=True, errors="replace")
        if r.returncode != 0:
            sys.stderr.write(r.stderr[-3000:])
            sys.stderr.write("ptrcheck64: clang failed on %s (its AST is still read)\n" % f)
            found.setdefault("ERROR %s" % f.split("/")[-1], ("clang errors", f))
        stack = []   # (depth, record name)
        curfile, curline = None, 0
        sext = None  # (depth, key) of an int -> pointer cast whose operand is next
        estack = []  # every node's (depth, kind, type): the ancestors of the current one
        for line in r.stdout.splitlines():
            m = DECL.match(line)
            g0 = ANY.match(line)
            if g0:
                d0 = len(g0.group(1))
                while estack and estack[-1][0] >= d0:
                    estack.pop()
                kind0 = re.match(r"^[ |`-]*(\w+)", line).group(1)
                t0 = first_type(line[g0.end():])
                # pointer arithmetic with a pointer cast to a signed int: an
                # N64 address >= 0x80000000 is negative and sign-extends
                # (`p += (s32) q`: N64_A32 instead of s32)
                if kind0 == "CStyleCastExpr" and "<PointerToIntegral>" in line and t0 in SIGNED32:
                    for d, kd, td in reversed(estack):
                        if kd in ("ParenExpr", "ImplicitCastExpr", "CStyleCastExpr"):
                            continue
                        if kd in ("BinaryOperator", "CompoundAssignOperator") and td and "*" in td:
                            lm2 = LOCTOK.findall(line.split(">")[0] if "<" in line else "")
                            ln = curline
                            for fl, a1, a2 in lm2:
                                ln = int(a1) if fl else int(a2) if a2 else ln
                                break
                            found.setdefault("PADD %s:%d" % ((curfile or "?").split("/")[-1], ln),
                                             (t0, "%s:%d" % (curfile, ln)))
                        break
                estack.append((d0, kind0, t0))
            if sext is not None:
                g = ANY.match(line)
                if g and len(g.group(1)) > sext[0]:
                    ot = first_type(line[g.end():])
                    # (a literal can't be negative here: no sign to extend)
                    if ot in SIGNED32 and not re.search(r"\bIntegerLiteral 0x", line):
                        found.setdefault(sext[1], (ot, sext[2]))
                    sext = None
            # clang prints a location relative to the previous one: follow it
            lpart = line.split(">")[0] if "<" in line else ""
            locs = LOCTOK.findall(lpart)
            first = None
            for fl, ln, ln2 in locs:
                if fl:
                    curfile, curline = fl, int(ln)
                elif ln2:
                    curline = int(ln2)
                if first is None:
                    first = (curfile, curline)
            # sizeof(T *) of a host-width pointer: 8 natively, 4 on the N64
            # (a table of N64 pointers sized with it comes out twice as big)
            sz = re.search(r"UnaryExprOrTypeTraitExpr 0x[0-9a-f]+ <[^>]*> '[^']*' sizeof '([^']*)'(?::'([^']*)')?", line)
            if sz:
                st = sz.group(2) or sz.group(1)
                if host_ptr(st) and not is_fnptr(st):
                    here = "%s:%d" % ((curfile or "?").split("/")[-1], curline)
                    found.setdefault("SIZEOF %s" % here, (st, "%s:%d" % (curfile, curline)))
            if not m:
                continue
            depth = len(m.group("indent"))
            kind, rest = m.group("kind"), m.group("rest")
            while stack and stack[-1][0] >= depth:
                stack.pop()
            where = first or (curfile, curline)
            if kind == "RecordDecl":
                nm = re.search(r"\b(struct|union) (\w+)", rest)
                if nm and nm.group(2) == "definition":
                    nm = None
                name = nm.group(2) if nm else "anon@%s:%d" % ((where[0] or "?").split("/")[-1], where[1])
                stack.append((depth, name))
                continue
            if kind == "TypedefDecl":
                continue
            t = first_type(rest)
            src = "%s:%d" % (where[0] or "?", where[1])
            if kind == "FieldDecl":
                fm = re.search(r"\b(\w+) '", rest)
                fname = fm.group(1) if fm else "?"
                rec = stack[-1][1] if stack else "?"
                if is_fnptr(t):
                    key = "FNPTR %s.%s" % (rec, fname)
                elif host_ptr(t):
                    key = "FIELD %s.%s" % (rec, fname)
                else:
                    continue
                found.setdefault(key, (t, src))
            elif kind == "VarDecl":
                vm = re.search(r"\b(\w+) '", rest)
                vname = vm.group(1) if vm else "?"
                if vname not in pinned and not re.match(r"D_[0-9A-F]{8}$", vname):
                    continue
                if host_ptr(t):
                    found.setdefault("VAR %s" % vname, (t, src))
            elif kind == "CStyleCastExpr":
                here = "%s:%d" % ((where[0] or "?").split("/")[-1], where[1])
                if ptr_to_host_ptr(t):
                    found.setdefault("CAST %s" % here, (t, src))
                # an int -> host-width pointer cast (to a __ptr32 one truncates:
                # nothing to extend)
                if "<IntegralToPointer>" in rest and t and t.count("__ptr32") < t.count("*"):
                    sext = (depth, "SEXT %s" % here, src)
    bad = 0
    kinds = {}
    with open(out_path, "w") as o:
        for k in sorted(found):
            t, fl = found[k]
            ok = allowed(k)
            if not ok:
                bad += 1
                kinds[k.split()[0]] = kinds.get(k.split()[0], 0) + 1
            o.write("%s%s\t%s\t%s\n" % ("allowed " if ok else "", k, t, fl))
    print("ptrcheck64: %d findings not allowed %s (%s)" % (bad, kinds, out_path))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
