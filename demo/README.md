# Interactive audio demo

Run this Godot project to compare seek, playback, and looping across the addon
and Godot's native audio streams. The stream picker always includes the bundled
Ogg Opus and 39 kHz FLAC examples.

To make local WAV, Ogg Vorbis, and MP3 versions of `sound.opus` for a
same-source UI comparison:

```bash
bash tools/make_demo_comparisons.sh
```

Those generated files are intentionally ignored: the WAV alone is large, and
transcoding an already-lossy source is useful for UI testing rather than audio
quality evaluation.

To download a focused FLAC conformance set covering 96 kHz/24-bit, 5.1,
unknown total length, and mono streams:

```bash
bash tools/fetch_flac_test_files.sh
```

Downloaded fixtures appear in the picker automatically. They come from the
[IETF CELLAR FLAC decoder testbench](https://github.com/ietf-wg-cellar/flac-test-files)
and are dedicated to the public domain under CC0 1.0. Multichannel samples are
useful compatibility tests; this addon currently presents the first two FLAC
channels through Godot's stereo `AudioFrame` playback interface rather than
performing a surround-aware downmix.
