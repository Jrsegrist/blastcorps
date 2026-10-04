#!/usr/bin/env python3
"""Post-process splat's nonmatching .s files for IDO's rodata layout.

IDO emits a file's .rodata as all string literals first (in function
order), then "late rodata" - float/double constants and jump tables - also
in function order. splat writes each GLOBAL_ASM function's rodata as one
`.section .rodata` block in address order, i.e. strings followed by
constants. asm-processor needs the constants in `.section .late_rodata`
(with `.late_rodata_alignment 8` when doubles are present), or it
mis-sizes the section and the layout breaks.

usage: late_rodata.py asm/nonmatchings/hd_code/FILE [...]
Idempotent: files already containing .late_rodata are left alone.
"""
import re
import sys
import glob
import os

# Late = float/double constants and jump tables (.word <label>). A numeric
# .word is a short string (splat writes "in\0\0" as .word 0x696E0000).
# If late data seems to sit between strings, the splat "file" is really two
# original files (26570 was yoshi.c + the scheduler): split it.
LATE = re.compile(r'^\s*\.(float|double)\b|^\s*\.word\s+[A-Za-z_.]')


def fix(path):
    s = open(path).read()
    if '.late_rodata' in s or '.section .rodata' not in s:
        return False
    head, sep, rest = s.partition('.section .rodata')
    # rodata block ends at the next .section directive (normally .text)
    m = re.search(r'^\.section \.text', rest, re.M)
    ro, tail = (rest[:m.start()], rest[m.start():]) if m else (rest, '')
    blocks = re.split(r'(?=^glabel )', ro, flags=re.M)
    early, late = [], []
    for b in blocks:
        body = [l for l in b.split('\n')[1:] if l.strip()]
        is_late = b.startswith('glabel') and body and LATE.match(body[0])
        is_str = b.startswith('glabel') and body and re.match(r'^\s*\.(ascii|asciz)\b', body[0])
        if is_late or (late and not is_str):
            late.append(b)
        else:
            early.append(b)
    if not late:
        return False
    align = ''
    if any('.double' in b for b in late):
        # the block's first constant tells where it really starts: a leading
        # .float at 4 mod 8 means the block is only 4-aligned
        m0 = re.match(r'glabel \w+_([0-9A-F]{8})', late[0])
        start = int(m0.group(1), 16) if m0 else 0
        align = '.late_rodata_alignment %d\n' % (4 if start % 8 else 8)
    out = head + sep + ''.join(early)
    out += '\n.section .late_rodata\n' + align + '\n' + ''.join(late)
    out += tail
    open(path, 'w').write(out)
    return True


def main():
    n = 0
    for d in sys.argv[1:]:
        for p in sorted(glob.glob(os.path.join(d, '*.s'))):
            n += fix(p)
    print('late_rodata: rewrote %d file(s)' % n)


if __name__ == '__main__':
    main()
