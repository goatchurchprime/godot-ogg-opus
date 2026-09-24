#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
NIXPKGS="${NIXPKGS:-github:NixOS/nixpkgs/b6018f87da91d19d0ab4cf979885689b469cdd41}"
SOURCE="$ROOT/demo/sound.opus"

if [[ ! -f "$SOURCE" ]]; then
    echo "Missing $SOURCE" >&2
    exit 1
fi

# These are deliberately local demo artifacts rather than release contents.
# WAV is large, and re-encoding a lossy Opus source is for UI/A-B testing only.
nix shell "${NIXPKGS}#ffmpeg" --command ffmpeg -hide_banner -loglevel warning -y -i "$SOURCE" -c:a pcm_s16le "$ROOT/demo/sound.wav"
nix shell "${NIXPKGS}#ffmpeg" --command ffmpeg -hide_banner -loglevel warning -y -i "$SOURCE" -c:a libvorbis -q:a 5 "$ROOT/demo/sound.ogg"
nix shell "${NIXPKGS}#ffmpeg" --command ffmpeg -hide_banner -loglevel warning -y -i "$SOURCE" -c:a libmp3lame -q:a 2 "$ROOT/demo/sound.mp3"

echo "Created WAV, Ogg Vorbis, and MP3 comparison files in demo/."
