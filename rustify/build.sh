#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

mkdir -p include build

echo "🔨 Compiling Rust static library..."
rustc --edition 2024 --crate-type staticlib --crate-name mylib -C opt-level=3 \
    -C lto -C codegen-units=1 -C panic=abort -o build/libmylib.a src/lib.rs

echo "🔨 Generating C header..."
cbindgen --config cbindgen.toml --output include/mylib.h src/lib.rs

echo "🔨 Compiling C program..."
gcc -O2 -Wall -Wextra -Iinclude -o build/program c/main.c build/libmylib.a -lm

echo "✅ Build complete! Running:"
echo "─────────────────────────────"
./build/program
