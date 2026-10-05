import os
import re

# Segment vram ranges, used to pick the segment from the function name.
SEGMENTS = [
    ('hd_front_end', 0x801E7000, 0x80210E90),
    ('init', 0x8021ED00, 0x80224A00),
]


def pick_segment(args):
    # DIFF_SEG=init|hd_code|hd_front_end overrides the guess.
    seg = os.environ.get('DIFF_SEG')
    if seg:
        return seg
    m = re.search(r'([0-9A-Fa-f]{8})', str(getattr(args, 'start', '') or ''))
    if m:
        addr = int(m.group(1), 16)
        for name, lo, hi in SEGMENTS:
            if lo <= addr < hi:
                return name
    return 'hd_code'


def apply(config, args):
    basename = pick_segment(args)
    if os.path.exists(f'build/{basename}.us.v10.bin'):
        version = 'us.v10'
    elif os.path.exists(f'build/{basename}.us.v11.bin'):
        version = 'us.v11'
    else:
        version = 'eu'

    config['baseimg'] = f'{basename}.{version}.bin'
    config['myimg'] = f'build/{basename}.{version}.bin'
    config['mapfile'] = f'build/{basename}.{version}.map'
    config['source_directories'] = ['src', f'src.{version}', 'include']
