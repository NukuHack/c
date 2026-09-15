#!/usr/bin/env bash
# Generates headers, compiles, and runs the program.
# Usage: ./build.sh          -> build + run
#        ./build.sh build    -> build only

set -e  # stop on first error

# Paths (relative to this script's location)
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$ROOT/c"
INC_DIR="$ROOT/include"
BUILD_DIR="$ROOT/build"
OUT="$BUILD_DIR/main.out"

# 1. Generate headers from every .c file
mkdir -p "$INC_DIR" "$BUILD_DIR"
echo "==> Generating headers..."
for src in "$SRC_DIR"/*.c; do
    [ -e "$src" ] || continue
    python "$ROOT/header_gen.py" "$src"
done

# 2. Compile everything
echo "==> Compiling..."
gcc -Wall -Wextra -g -I"$INC_DIR" -o "$OUT" "$SRC_DIR"/*.c

# 3. Run (unless 'build' was passed)
if [ "$1" != "build" ]; then
    echo "==> Running $OUT"
    echo "----------------------------------------"
    "$OUT"
    echo "----------------------------------------"
fi