# Godot Ogg Opus

A small GDExtension that makes `.opus` (Ogg Opus) files first-class Godot
`AudioStream` resources. It uses Xiph's `libopusfile`, `libopus`, and `libogg`.
It is independent of TwoVoIP: this addon handles stored Ogg Opus media, while
TwoVoIP handles live raw Opus packets.

```gdscript
var stream: AudioStreamOggOpus = load("res://voice.opus")
$AudioStreamPlayer.stream = stream
$AudioStreamPlayer.play()
```

The resource supports duration reporting, playback, seeking, simultaneous
playbacks, and optional looping with a loop offset. The compressed bytes stay
inside the resource and are decoded incrementally; the complete PCM file is
never expanded into memory.

## Install

Download `godot-ogg-opus.zip` from a release and extract it at the root of a
Godot project. It contains only `addons/ogg_opus`. The extension is loaded
automatically; there is no EditorPlugin to enable.

Release archives support Linux and Windows x86-64, universal macOS, iOS arm64,
all four Android architectures, and threaded or single-threaded Web exports.

## Build and test on NixOS

```bash
git submodule update --init --recursive
bash tools/build_linux.sh
bash tools/smoke_linux.sh
```

The local scripts use CMake for a quick Linux development build. Distribution
builds use SCons, matching Godot's platform and architecture options:

```bash
python -m pip install scons
scons build_ogg platform=linux target=template_debug arch=x86_64
scons build_opus platform=linux target=template_debug arch=x86_64
scons platform=linux target=template_debug arch=x86_64
```

The native dependencies are statically linked into one GDExtension binary.
No system Opus/Ogg installation is required at runtime.

## Releases and Asset Library packaging

Every GitHub Actions build produces a `godot-ogg-opus.zip` artifact containing
the complete addon and all platform binaries. A tag publishes that same archive
to a GitHub release. Use the release archive URL—not GitHub's source archive—as
the Asset Library download URL, because source archives do not contain built
binaries and do not recursively include submodules.

## Status

The minimum supported engine version is Godot 4.5.

Suggested Asset Library fields are **Godot Ogg Opus**, category
**Addons / Audio**, license **MIT**, and minimum Godot version **4.5**. The square icon is
`addons/ogg_opus/icon.png`; its direct GitHub URL can be used for the listing.

Third-party sources retain their own licences: Opus (BSD), opusfile (BSD),
libogg (BSD), and godot-cpp (MIT).
