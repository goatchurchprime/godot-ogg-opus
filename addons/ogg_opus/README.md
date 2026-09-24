# Godot Ogg Opus

This GDExtension makes Ogg Opus (`.opus`) files first-class Godot audio
resources on desktop, mobile, and the web.

```gdscript
var stream: AudioStreamOggOpus = load("res://voice.opus")
$AudioStreamPlayer.stream = stream
$AudioStreamPlayer.play()
```

Copy this `ogg_opus` directory into your project's `addons` directory. No
EditorPlugin needs to be enabled: Godot loads the GDExtension automatically.

The resource supports duration reporting, playback, seeking, simultaneous
playbacks, and optional looping with a loop offset. Decoding is incremental;
the entire PCM stream is not expanded into memory.

Supported targets are Linux and Windows x86-64, universal macOS, iOS arm64,
Android arm32/arm64/x86-32/x86-64, and threaded or single-threaded Web builds.

Source, issues, and build instructions:
https://github.com/goatchurchprime/godot-ogg-opus

## AI assistance

OpenAI Codex was used as a coding agent to discuss the addon's architecture,
implement substantial portions of the initial GDExtension, add cross-platform
build and release packaging, and help diagnose build failures. Julian Todd
directed and reviewed the work and remains responsible for the project and its
published releases.
