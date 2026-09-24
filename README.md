# Xiph Audio for Godot

A cross-platform GDExtension that makes Xiph.Org audio formats first-class
Godot `AudioStream` resources. It currently supports Ogg Opus (`.opus`) through
`AudioStreamOggOpus` and FLAC (`.flac`) through `AudioStreamFLAC`.

```gdscript
var voice := load("res://voice.opus") as AudioStreamOggOpus
var music := load("res://music.flac") as AudioStreamFLAC
$AudioStreamPlayer.stream = music
$AudioStreamPlayer.play()
```

Both resources support duration reporting, playback, seeking, simultaneous
playbacks, and optional looping with a loop offset. Compressed bytes remain in
the resource and are decoded incrementally, so FLAC conserves project and
resident compressed-data memory instead of being expanded into a complete WAV
buffer by this addon.

This project is independent of [TwoVoIP](https://github.com/goatchurchprime/two-voip-godot-4),
which handles live raw Opus packets. Xiph Audio handles stored audio files. It
provides decoding/playback only; it does not replace Godot's `save_to_wav()`
with audio conversion or encoding APIs.

## Install

Download `godot-xiph-audio.zip` from a release and extract it at the root of a
Godot project. It contains only `addons/xiph_audio`. The GDExtension loads
automatically; there is no EditorPlugin to enable.

Release archives support Linux and Windows x86-64, universal macOS, iOS arm64,
all four Android architectures, and threaded or single-threaded Web exports.
The minimum supported engine version is Godot 4.5.

## Migrating from Godot Ogg Opus

The repository and Asset Library entry are now named **Xiph Audio for Godot**.
Replace `addons/ogg_opus` with `addons/xiph_audio`; do not keep both copies.
Existing `.opus` files and `AudioStreamOggOpus` scripts/resources remain
compatible. The native filename and descriptor changed internally and require
no script changes.

GitHub redirects links and clones after a repository rename. Existing local
clones can update their remote explicitly:

```bash
git remote set-url origin https://github.com/goatchurchprime/godot-xiph-audio.git
```

## Build and test on NixOS

```bash
git submodule update --init --recursive
bash tools/build_linux.sh
bash tools/smoke_linux.sh
```

Distribution builds use SCons:

```bash
python -m pip install scons
scons build_ogg platform=linux target=template_debug arch=x86_64
scons build_opus platform=linux target=template_debug arch=x86_64
scons build_flac platform=linux target=template_debug arch=x86_64
scons platform=linux target=template_debug arch=x86_64
```

All codec dependencies are statically linked into one GDExtension binary.

## Releases and Asset Library

Each GitHub Actions build produces `godot-xiph-audio.zip`. Tags publish the
same archive to a GitHub release. Use that release archive—not GitHub's source
archive—as the Asset Library download URL, since source archives omit compiled
binaries and submodule contents.

Suggested fields are **Xiph Audio for Godot**, category **Addons / Audio**,
license **MIT**, and minimum Godot version **4.5**. The listing icon is
`addons/xiph_audio/icon.png`.

Third-party sources retain their licenses: FLAC, Opus, opusfile, and libogg
(BSD-style), and godot-cpp (MIT). See the packaged notices for details.

## AI assistance

OpenAI Codex was used as a coding agent to discuss scope, implement substantial
portions of the GDExtension and FLAC support, add cross-platform build and
release packaging, and diagnose build failures. Julian Todd directed and
reviewed the work and remains responsible for the project and its releases.
