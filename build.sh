#!/usr/bin/env bash
# Generates headers, compiles, and runs the program.
# Usage: ./build.sh          -> build + run
#        ./build.sh build    -> build only

set -e

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INC_DIR="$ROOT/include"
BUILD_DIR="$ROOT/build"
OUT="$BUILD_DIR/main.out"

# Auto-detect the source directory (base/, basic/, or src/)
for d in base basic src; do
    if [ -d "$ROOT/$d" ]; then
        SRC_DIR="$ROOT/$d"
        break
    fi
done
if [ -z "$SRC_DIR" ]; then
    echo "Error: no source directory (base/, basic/, or src/) under $ROOT" >&2
    exit 1
fi

# Collect sources
shopt -s nullglob
sources=("$SRC_DIR"/*.c)
shopt -u nullglob
if [ ${#sources[@]} -eq 0 ]; then
    echo "Error: no .c files in $SRC_DIR" >&2
    exit 1
fi

# 1. Generate headers (single call, all files)
mkdir -p "$INC_DIR" "$BUILD_DIR"
echo "==> Generating headers..."
python "$ROOT/header_gen.py" "${sources[@]}"

# 2. Compile
echo "==> Compiling..."
gcc -Wall -Wextra -g -I"$INC_DIR" -o "$OUT" "${sources[@]}"

# 3. Run (unless 'build' was passed)
if [ "$1" != "build" ]; then
    echo "==> Running $OUT"
    echo "----------------------------------------"
    "$OUT"
    echo "----------------------------------------"
fi