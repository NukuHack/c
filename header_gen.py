#!/usr/bin/env python3
"""
A simple header generator for C.
Scans .c files for doc comments containing '@public' and generates a .h file
with the function prototypes and their documentation.
"""
import os
import re
import sys

# Regex to find a doc comment block immediately followed by a function definition.
# It captures the comment, the function signature, and the opening brace.
FUNCTION_PATTERN = re.compile(
    r'/\*\*(?P<comment>.*?)\*/\s*'  # The /** ... */ comment
    r'(?P<signature>[a-zA-Z_][a-zA-Z0-9_*\s]+\([^)]*\))\s*\{',  # The function signature
    re.DOTALL
)

def generate_header(source_file, header_file):
    """Parses a .c file and writes a .h file with exported functions."""
    print(f"Generating {header_file} from {source_file}...")
    with open(source_file, 'r') as f:
        content = f.read()

    exported_functions = []
    for match in FUNCTION_PATTERN.finditer(content):
        comment = match.group('comment').strip()
        signature = match.group('signature').strip()

        # Check if the '@public' annotation is in the comment
        if '@public' in comment:
            # Reconstruct the doc comment for the header
            doc_comment = f"/**\n * {comment.replace('\n', '\n * ')}\n */"
            # Ensure the signature ends with a semicolon
            prototype = signature if signature.endswith(';') else signature + ';'
            exported_functions.append(f"{doc_comment}\n{prototype}")

    # Write the header file
    # Build a clean include guard from the header's basename only.
    # Any character that isn't a letter, digit, or underscore becomes '_'.
    guard = re.sub(r'[^A-Za-z0-9]', '_', os.path.basename(header_file)).upper()

    # Write the header file
    with open(header_file, 'w') as f:
        f.write(f"// Auto-generated header for {os.path.basename(source_file)}\n")
        f.write(f"#ifndef {guard}\n")
        f.write(f"#define {guard}\n\n")
        if exported_functions:
            f.write("\n\n".join(exported_functions))
        f.write("\n\n#endif\n")

    print(f"Done. Found {len(exported_functions)} exported function(s).")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python generate_headers.py <file1.c> [file2.c ...]")
        sys.exit(1)

    # All generated headers go here, next to the 'c' folder.
    include_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "include")
    os.makedirs(include_dir, exist_ok=True)

    for source in sys.argv[1:]:
        if source.endswith('.c'):
            basename = os.path.basename(source)          # e.g. math_utils.c
            header = basename.replace('.c', '.h')        # e.g. math_utils.h
            header_path = os.path.join(include_dir, header)
            generate_header(source, header_path)