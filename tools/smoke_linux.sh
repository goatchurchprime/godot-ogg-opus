#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NIXPKGS="${NIXPKGS:-github:NixOS/nixpkgs/b6018f87da91d19d0ab4cf979885689b469cdd41}"

mkdir -p "$ROOT/demo/addons"
rm -f "$ROOT/demo/addons/ogg_opus"
ln -sfn ../../addons/xiph_audio "$ROOT/demo/addons/xiph_audio"
rm -f "$ROOT/demo/.godot/extension_list.cfg"
nix shell "${NIXPKGS}#ffmpeg" --command ffmpeg -hide_banner -loglevel error \
    -f lavfi -i "sine=frequency=440:duration=1" -ac 1 -c:a libopus -y "$ROOT/demo/fixture.opus"
nix shell "${NIXPKGS}#ffmpeg" --command ffmpeg -hide_banner -loglevel error \
    -f lavfi -i "sine=frequency=660:duration=1" -ac 1 -c:a flac -y "$ROOT/demo/fixture.flac"
# Refresh extension discovery after addon installs, removals, or renames.
nix shell "${NIXPKGS}#godot_4_6" --command godot4 \
    --headless --editor --path "$ROOT/demo" --quit-after 2
nix shell "${NIXPKGS}#godot_4_6" --command godot4 \
    --headless --path "$ROOT/demo" res://smoke.tscn
