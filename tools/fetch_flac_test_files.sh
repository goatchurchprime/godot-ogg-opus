#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="$ROOT/demo/test_files"
BASE="https://raw.githubusercontent.com/ietf-wg-cellar/flac-test-files/main/subset"

mkdir -p "$DEST"
files=(
    "28 - high resolution audio, default settings.flac"
    "41 - 6 channels (5.1).flac"
    "45 - no total number of samples set.flac"
    "60 - mono audio.flac"
)

for filename in "${files[@]}"; do
    encoded_filename="${filename// /%20}"
    curl --fail --location --retry 3 "$BASE/$encoded_filename" --output "$DEST/$filename"
done

echo "Downloaded CC0 FLAC decoder testbench samples to $DEST."
