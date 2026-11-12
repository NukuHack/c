#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
header_gen.py - Auto-generate C headers from .c files.

Compatible with Python 2.7 and 3.x.

Scans each .c file for doc comments that contain the '@public' tag and
emits a matching .h file containing the documented prototypes.

Usage:
    python header_gen.py <file1.c> [file2.c ...]
"""
from __future__ import print_function, unicode_literals

import io
import os
import re
import sys


# /*** ... ***/  immediately followed by  <return type> <name>(<params>)  {
FUNCTION_RE = re.compile(
    r'/\*\*(?P<comment>.*?)\*/\s*'
    r'(?P<signature>'
        r'[A-Za-z_][A-Za-z0-9_*\s]*'   # return type (may contain ' * ')
        r'[A-Za-z_][A-Za-z0-9_]*'      # function name
        r'\s*\([^;{}]*\)'              # parameter list
    r')'
    r'\s*\{',
    re.DOTALL,
)

# '@public' as a standalone token
PUBLIC_RE = re.compile(r'(?<!\S)@public(?!\S)')


def normalize_comment(raw):
    """Turn the text between /** and */ into a list of clean, unstarred lines."""
    lines = []
    for line in raw.splitlines():
        line = line.strip()
        if line.startswith('*'):
            line = line[1:]
            if line.startswith(' '):
                line = line[1:]
        lines.append(line.rstrip())
    # Drop leading / trailing blank lines
    while lines and not lines[0]:
        lines.pop(0)
    while lines and not lines[-1]:
        lines.pop()
    return lines


def has_public(lines):
    return any(PUBLIC_RE.search(line) for line in lines)


def render_doc(lines):
    """Re-emit the cleaned lines as a well-formed /** ... */ block."""
    out = ['/**']
    for line in lines:
        out.append(' * ' + line if line else ' *')
    out.append(' */')
    return '\n'.join(out)


def generate_header(source_file, header_file):
    print("Generating {0} from {1}...".format(header_file, source_file))

    with io.open(source_file, 'r', encoding='utf-8') as f:
        content = f.read()

    prototypes = []
    for match in FUNCTION_RE.finditer(content):
        body = normalize_comment(match.group('comment'))
        if not has_public(body):
            continue

        # Drop the '@public' marker from the emitted documentation.
        body = [PUBLIC_RE.sub('', line).strip() for line in body]
        while body and not body[0]:
            body.pop(0)
        while body and not body[-1]:
            body.pop()

        # Collapse whitespace in multi-line signatures onto one line.
        signature = ' '.join(match.group('signature').split())
        if not signature.endswith(';'):
            signature += ';'

        prototypes.append(render_doc(body) + '\n' + signature)

    # Clean include guard from header basename.
    guard = re.sub(r'[^A-Za-z0-9]', '_', os.path.basename(header_file)).upper()
    if guard and guard[0].isdigit():
        guard = '_' + guard

    with io.open(header_file, 'w', encoding='utf-8') as f:
        f.write("// Auto-generated header for {0}\n".format(
            os.path.basename(source_file)))
        f.write("#ifndef {0}\n".format(guard))
        f.write("#define {0}\n\n".format(guard))
        if prototypes:
            f.write('\n\n'.join(prototypes))
            f.write('\n')
        f.write("\n#endif // {0}\n".format(guard))

    print("Done. Found {0} exported function(s).".format(len(prototypes)))


def main(argv):
    if len(argv) < 2:
        print("Usage: python header_gen.py <file1.c> [file2.c ...]")
        return 1

    here = os.path.dirname(os.path.abspath(__file__))
    include_dir = os.path.join(here, 'include')
    try:
        os.makedirs(include_dir)
    except OSError:
        if not os.path.isdir(include_dir):
            raise

    for path in argv[1:]:
        if not path.endswith('.c'):
            continue
        if not os.path.isfile(path):
            print("Skipping missing file: {0}".format(path))
            continue
        header_name = os.path.basename(path)[:-2] + '.h'
        generate_header(path, os.path.join(include_dir, header_name))

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))