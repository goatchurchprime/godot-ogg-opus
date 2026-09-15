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

## Build and test on NixOS

```bash
git submodule update --init --recursive
bash tools/build_linux.sh
bash tools/smoke_linux.sh
```

The native dependencies are statically linked into one GDExtension binary.
No system Opus/Ogg installation is required at runtime.

## Try a built component

Every GitHub Actions build produces a `godot-ogg-opus-linux-x86_64.zip`
artifact containing the complete `addons/ogg_opus` directory. Copy that
directory into an existing Godot 4.5-or-newer project, then load any `.opus`
file as shown above. Tagged builds attach the same archive to a GitHub release.

## Status

This is an early interest-testing build. Linux x86-64 is the first verified
and packaged target. Windows, macOS, Android and Web packaging remain before
an Asset Library submission.

Third-party sources retain their own licences: Opus (BSD), opusfile (BSD),
libogg (BSD), and godot-cpp (MIT).
