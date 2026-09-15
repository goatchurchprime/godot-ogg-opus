#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NIXPKGS="${NIXPKGS:-github:NixOS/nixpkgs/b6018f87da91d19d0ab4cf979885689b469cdd41}"

cd "$ROOT"
nix shell "${NIXPKGS}#cmake" "${NIXPKGS}#ninja" "${NIXPKGS}#gcc" \
    --command cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug -DGODOTCPP_TARGET=template_debug
nix shell "${NIXPKGS}#cmake" "${NIXPKGS}#ninja" "${NIXPKGS}#gcc" \
    --command cmake --build build --parallel

