# Godot Asset Store listing

## Core fields

- Name: `Xiph Audio for Godot`
- Version: `0.2.0`
- License: `MIT`
- Godot version: `4.5 or newer`
- Repository: `https://github.com/goatchurchprime/godot-xiph-audio`
- Download: `https://github.com/goatchurchprime/godot-xiph-audio/releases/download/v0.2.0/godot-xiph-audio.zip`
- Demo: `https://goatchurch.itch.io/godot-xiph-audio-demo`
- Thumbnail: `docs/asset-store-thumbnail.png`
- Tags: `Audio`, `Opus`, `FLAC`, `GDExtension`, `Cross-platform`

## Summary

Play Ogg Opus and FLAC files as native Godot AudioStream resources on desktop,
mobile, and Web.

## Description

Xiph Audio for Godot is a cross-platform Godot 4 GDExtension for loading and
playing Ogg Opus (`.opus`) and FLAC (`.flac`) files as native AudioStream
resources.

It supports duration reporting, seeking, simultaneous playback, and optional
looping with a loop offset. Audio is decoded incrementally from compressed
resource data. The addon loads automatically and does not require enabling an
EditorPlugin.

The release includes debug and release binaries for Linux and Windows x86-64,
universal macOS, iOS arm64, Android arm32/arm64/x86-32/x86-64, and threaded or
single-threaded Web exports. Godot 4.5 or newer is required.

This addon provides decoding and playback only; it does not provide audio
encoding or format conversion. Source, documentation, and issue tracking are
available from the GitHub repository. A live WebAssembly demo is available on
itch.io.

## AI-use disclosure

OpenAI Codex was used as a coding agent to discuss scope, implement substantial
portions of the GDExtension and FLAC support, add cross-platform build and
release packaging, and diagnose build failures. Julian Todd directed and
reviewed the work and remains responsible for the project and its releases.
