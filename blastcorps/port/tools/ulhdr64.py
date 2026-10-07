#!/usr/bin/env python3
"""ultralib's headers with the N64 pointer marks, for the 64-bit build's ul_* objects.

The ul_* objects (libultra/libaudio compiled natively) see ultralib's own
headers, not the game's include/2.0I ones; the records they share with the game
(and the libaudio records kept in the audio heap in RDRAM) must keep their N64
layout, so their pointer fields get N64P / N64FN here, in copies under OUT
(found before UL_DIR on the include path).  The submodule is never edited.

usage: ulhdr64.py UL_DIR LIST OUT
  LIST lines:  PATH LINE FIELD                 insert N64P after every '*' of the
                                               declaration on that line (it must
                                               name FIELD)
               PATH LINE FIELD fn TYPEDEF      a function pointer field: becomes
                                               `N64FN(TYPEDEF) FIELD;`
  PATH is relative to UL_DIR (include/PR/libaudio.h, src/audio/synthInternals.h);
  copies keep that path under OUT.
"""
import os
import re
import sys


def star_insert(line):
    out, i, n, changed = [], 0, len(line), 0
    in_c = False
    while i < n:
        if in_c:
            j = line.find("*/", i)
            if j < 0:
                out.append(line[i:])
                break
            out.append(line[i:j + 2])
            i, in_c = j + 2, False
            continue
        if line.startswith("/*", i):
            out.append("/*")
            i, in_c = i + 2, True
            continue
        if line.startswith("//", i):
            out.append(line[i:])
            break
        if line[i] == "*" and not re.match(r"\s*N64P\b", line[i + 1:]):
            out.append("* N64P ")
            changed += 1
        else:
            out.append(line[i])
        i += 1
    return "".join(out), changed


def main():
    ul, lst, outdir = sys.argv[1:4]
    edits = {}
    for raw in open(lst):
        raw = raw.split("#")[0].strip()
        if not raw:
            continue
        p = raw.split()
        edits.setdefault(p[0], []).append(p[1:])
    # every header of include/ and src/ (so a header's quoted includes of its
    # neighbours find the marked copies too)
    for top in ("include", "src"):
        for d, _, fs in os.walk(os.path.join(ul, top)):
            for fn in fs:
                if fn.endswith(".h"):
                    rel = os.path.relpath(os.path.join(d, fn), ul).replace(os.sep, "/")
                    if rel in edits:
                        continue
                    dst = os.path.join(outdir, rel)
                    os.makedirs(os.path.dirname(dst), exist_ok=True)
                    with open(os.path.join(ul, rel), "rb") as a, open(dst, "wb") as b:
                        b.write(a.read())
    for path, es in sorted(edits.items()):
        src = open(os.path.join(ul, path), newline="").read().split("\n")
        for e in es:
            ln, field = int(e[0]), e[1]
            line = src[ln - 1]
            if not re.search(r"\b%s\b" % re.escape(field), line):
                sys.exit("ulhdr64: %s:%d doesn't declare %s: %s" % (path, ln, field, line.strip()))
            if len(e) > 2 and e[2] == "fn":
                m = re.match(r"^(\s*)", line)
                tail = re.search(r";(.*)$", line)
                src[ln - 1] = "%sN64FN(%s) %s;%s" % (m.group(1), e[3], field, tail.group(1) if tail else "")
            else:
                new, k = star_insert(line)
                if not k:
                    sys.exit("ulhdr64: %s:%d has no pointer: %s" % (path, ln, line.strip()))
                src[ln - 1] = new
        dst = os.path.join(outdir, path)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        with open(dst, "w", newline="") as f:
            f.write("\n".join(src))
    print("ulhdr64: %d headers" % len(edits))


if __name__ == "__main__":
    main()
