#!/usr/bin/env python3
"""runchecks: the eqcheck regression suite for NON_MATCHING rewrites.

Check specs live in tools_port/checks/<FILE>.txt, one file per source file
(src.us.v11/hd_code/<FILE>.c).  Each non-blank, non-comment line is

    func_XXXXXXXX: <eqcheck.py arguments>

and is one eqcheck run; a function may have any number of lines (a random run
plus boundary runs).  A line ending in a backslash continues on the next line.
'#' starts a comment line.  Arguments are split shell-style (shlex), but there
is no variable expansion.

Run from the repo dir with the venv active (or via tools_port/runchecks.sh):

    python3 tools_port/runchecks.py [-b] [-j N] [FILTER ...]

FILTER is a check-file name (8A080), a function name, or a substring of
either.  With no filter every check runs, and the coverage check also runs:
every function rewritten under #ifdef NON_MATCHING in src.us.v11/hd_code must
have at least one check line.  Exit status is nonzero if any check fails or
errors, or (unfiltered) if coverage is incomplete.
"""
import argparse
import concurrent.futures
import glob
import os
import re
import shlex
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
CHECKS = os.path.join(HERE, "checks")
SRC = os.path.join(REPO, "src.us.v11", "hd_code")


def parse_checks(path):
    """-> list of (file, lineno, func, argv)"""
    out = []
    fname = os.path.splitext(os.path.basename(path))[0]
    with open(path) as f:
        lines = f.read().split("\n")
    i = 0
    while i < len(lines):
        lineno = i + 1
        line = lines[i]
        i += 1
        while line.rstrip().endswith("\\") and i < len(lines):
            line = line.rstrip()[:-1] + " " + lines[i]
            i += 1
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        m = re.match(r"^([A-Za-z_]\w*)\s*:\s*(.*)$", s)
        if not m:
            sys.exit("%s:%d: expected 'func_XXXXXXXX: <eqcheck args>'" % (path, lineno))
        try:
            argv = shlex.split(m.group(2))
        except ValueError as e:
            sys.exit("%s:%d: %s" % (path, lineno, e))
        out.append((fname, lineno, m.group(1), argv))
    return out


def nm_functions():
    """func name -> source file stem, for every function whose asm sits in the
    #else branch of an #ifdef NON_MATCHING block."""
    funcs = {}
    for path in sorted(glob.glob(os.path.join(SRC, "*.c"))):
        stem = os.path.splitext(os.path.basename(path))[0]
        stack = []          # per open #if: [is_nm, in_else]
        with open(path, errors="replace") as f:
            for line in f:
                s = line.strip()
                if re.match(r"#\s*if(n?def)?\b", s):
                    stack.append([bool(re.match(r"#\s*ifdef\s+NON_MATCHING\b", s)
                                       or re.match(r"#\s*if\s+defined\s*\(?\s*NON_MATCHING", s)), False])
                elif re.match(r"#\s*else\b", s) and stack:
                    stack[-1][1] = True
                elif re.match(r"#\s*endif\b", s) and stack:
                    stack.pop()
                else:
                    m = re.search(r'GLOBAL_ASM\(\s*"([^"]+)"', s)
                    if m and any(nm and el for nm, el in stack):
                        funcs[os.path.splitext(os.path.basename(m.group(1)))[0]] = stem
    return funcs


def run_one(check, trials_override):
    fname, lineno, func, argv = check
    argv = list(argv)
    if trials_override is not None:
        # cap the trial count (quick mode)
        if "-n" in argv:
            k = argv.index("-n")
            argv[k + 1] = str(min(int(argv[k + 1]), trials_override))
        else:
            argv += ["-n", str(trials_override)]
    cmd = [sys.executable, os.path.join(HERE, "eqcheck.py"), func] + argv
    t0 = time.time()
    p = subprocess.run(cmd, cwd=REPO, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                       universal_newlines=True)
    dt = time.time() - t0
    out = p.stdout
    m = re.search(r"^(PASS|FAIL): \S+ (\d+)/(\d+) trials equivalent(.*?) \(", out, re.M)
    if p.returncode == 0 and m and m.group(1) == "PASS":
        status = "PASS"
    elif m and m.group(1) == "FAIL":
        status = "FAIL"
    else:
        status = "ERROR"
    summary = ("%s/%s%s" % (m.group(2), m.group(3), m.group(4).replace(" where both faulted identically",
                                                                       " both-faulted"))) if m else ""
    if "warning: every trial faulted" in out:
        status = "FAIL"
        summary += " (every trial faulted)"
    return check, status, summary, dt, out, cmd


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("filters", nargs="*", help="check file (8A080), function name, or substring")
    ap.add_argument("-b", "--build", action="store_true", help="make NON_MATCHING=1 first")
    ap.add_argument("-B", "--build-both", action="store_true",
                    help="make the matching build (must print both OK lines) and NON_MATCHING=1 first")
    # measured under WSL: throughput stops improving at about 4 parallel runs
    # (each eqcheck process is heavy on memory copies / sys time)
    ap.add_argument("-j", "--jobs", type=int, default=min(os.cpu_count() or 4, 8))
    ap.add_argument("-q", "--quick", type=int, metavar="N", help="cap every run at N trials")
    ap.add_argument("-v", "--verbose", action="store_true", help="print eqcheck output of every run")
    ap.add_argument("--list", action="store_true", help="list the selected checks and exit")
    ap.add_argument("--coverage-only", action="store_true", help="only run the coverage check")
    o = ap.parse_args()

    checks = []
    for path in sorted(glob.glob(os.path.join(CHECKS, "*.txt"))):
        checks += parse_checks(path)

    # coverage: every NON_MATCHING rewrite has a check line
    nm = nm_functions()
    covered = set(c[2] for c in checks)
    missing = sorted((stem, f) for f, stem in nm.items() if f not in covered)
    stale = sorted(set((c[0], c[2]) for c in checks if c[2] not in nm))
    cov_ok = not missing
    if not o.filters or o.coverage_only:
        print("coverage: %d NON_MATCHING function(s) in src.us.v11/hd_code, %d with checks"
              % (len(nm), len(nm) - len(missing)))
        for stem, f in missing:
            print("  MISSING  %s (%s.c): no line in tools_port/checks/" % (f, stem))
        for stem, f in stale:
            print("  note: checks/%s.txt tests %s, which has no #ifdef NON_MATCHING rewrite" % (stem, f))
        for c in checks:
            if c[2] in nm and nm[c[2]] != c[0]:
                print("  note: %s is in %s.c but checked in checks/%s.txt" % (c[2], nm[c[2]], c[0]))
        if o.coverage_only:
            return 0 if cov_ok else 1

    if o.filters:
        checks = [c for c in checks if any(f == c[0] or f == c[2] or f in c[0] or f in c[2]
                                           for f in o.filters)]
        if not checks:
            sys.exit("runchecks: no checks match %s" % " ".join(o.filters))
    if o.list:
        for c in checks:
            print("%s:%d  %s %s" % (c[0], c[1], c[2], " ".join(shlex.quote(a) for a in c[3])))
        return 0

    if o.build or o.build_both:
        steps = []
        if o.build_both:
            steps.append(("matching build", ["make", "VERSION=us.v11", "-j%d" % o.jobs]))
        steps.append(("NON_MATCHING build", ["make", "VERSION=us.v11", "NON_MATCHING=1", "-j%d" % o.jobs]))
        for title, cmd in steps:
            print("### %s: %s" % (title, " ".join(cmd)))
            sys.stdout.flush()
            p = subprocess.run(cmd, cwd=REPO, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               universal_newlines=True)
            if p.returncode != 0:
                print("\n".join(p.stdout.splitlines()[-40:]))
                sys.exit("runchecks: %s failed" % title)
            if title == "matching build":
                oks = [l for l in p.stdout.splitlines() if l.endswith(": OK")]
                for l in oks:
                    print("  " + l)
                if len(oks) < 2 and "Nothing to be done" not in p.stdout:
                    print("\n".join(p.stdout.splitlines()[-20:]))
                    sys.exit("runchecks: matching build did not print both OK lines")
        # every data symbol must keep its original address in build_nm
        p = subprocess.run([sys.executable, os.path.join(HERE, "nm_symaudit.py")], cwd=REPO,
                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
        print(p.stdout.rstrip())
        if p.returncode != 0:
            sys.exit("runchecks: data symbols moved in the NON_MATCHING build (see above)")

    print("running %d check(s) on %d job(s)%s" % (len(checks), o.jobs,
                                                 ", capped at %d trials" % o.quick if o.quick else ""))
    sys.stdout.flush()
    t0 = time.time()
    results = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, o.jobs)) as ex:
        futs = [ex.submit(run_one, c, o.quick) for c in checks]
        for fu in concurrent.futures.as_completed(futs):
            results.append(fu.result())
    order = {id(c): i for i, c in enumerate(checks)}
    results.sort(key=lambda r: order[id(r[0])])

    nfail = 0
    print()
    print("%-6s %-14s %-5s %-6s %-24s %6s" % ("file", "function", "line", "result", "trials", "time"))
    print("-" * 66)
    for (fname, lineno, func, argv), status, summary, dt, out, cmd in results:
        print("%-6s %-14s %-5d %-6s %-24s %5.1fs" % (fname, func, lineno, status, summary, dt))
        if status != "PASS":
            nfail += 1
        if o.verbose or status != "PASS":
            show = out.splitlines()
            if status != "PASS" and not o.verbose:
                show = [l for l in show if not l.startswith("eqcheck ")][:20]
            for l in show:
                print("    | " + l)
            if status != "PASS":
                print("    | rerun: bash tools_port/eq.sh %s" % " ".join(shlex.quote(a) for a in cmd[2:]))
    print("-" * 66)
    print("%d run(s): %d passed, %d failed (%.1fs wall)" % (len(results), len(results) - nfail, nfail,
                                                          time.time() - t0))
    if not o.filters and not cov_ok:
        print("coverage: %d NON_MATCHING function(s) have no checks (see MISSING above)" % len(missing))
    return 1 if nfail or (not o.filters and not cov_ok) else 0


if __name__ == "__main__":
    sys.exit(main())
