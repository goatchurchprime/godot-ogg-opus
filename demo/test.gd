extends Control

const TRACKS := [
	["Ogg Opus — demo soundtrack", "res://sound.opus"],
	["FLAC — 39 kHz conformance sample", "res://20 - samplerate 39kHz.flac"],
	["WAV — same-track comparison", "res://sound.wav"],
	["Ogg Vorbis — same-track comparison", "res://sound.ogg"],
	["MP3 — same-track comparison", "res://sound.mp3"],
]

@onready var player: AudioStreamPlayer = $AudioStreamPlayer
@onready var track_picker: OptionButton = $Panel/Margin/VBox/TrackRow/TrackPicker
@onready var play_button: Button = $Panel/Margin/VBox/Transport/Play
@onready var loop_button: CheckButton = $Panel/Margin/VBox/Transport/Loop
@onready var seek_slider: HSlider = $Panel/Margin/VBox/SeekRow/Seek
@onready var elapsed_label: Label = $Panel/Margin/VBox/SeekRow/Elapsed
@onready var duration_label: Label = $Panel/Margin/VBox/SeekRow/Duration
@onready var details_label: Label = $Panel/Margin/VBox/Details

var available_paths: Array[String] = []
var paused_position := 0.0
var dragging_seek := false

func _ready() -> void:
	for track in TRACKS:
		if ResourceLoader.exists(track[1], "AudioStream"):
			track_picker.add_item(track[0])
			available_paths.append(track[1])
	_add_downloaded_flac_tests()
	track_picker.item_selected.connect(_select_track)
	play_button.pressed.connect(_toggle_playback)
	loop_button.toggled.connect(_set_loop)
	seek_slider.drag_started.connect(func() -> void: dragging_seek = true)
	seek_slider.drag_ended.connect(_seek_finished)
	player.finished.connect(_playback_finished)
	if available_paths.is_empty():
		details_label.text = "No demo audio files were found."
		play_button.disabled = true
		seek_slider.editable = false
	else:
		_select_track(0)

func _add_downloaded_flac_tests() -> void:
	var directory := DirAccess.open("res://test_files")
	if directory == null:
		return
	var files := directory.get_files()
	files.sort()
	for filename in files:
		if filename.get_extension().to_lower() != "flac":
			continue
		track_picker.add_item("FLAC test — " + filename.get_basename())
		available_paths.append("res://test_files/" + filename)

func _process(_delta: float) -> void:
	if player.playing and not dragging_seek:
		seek_slider.value = player.get_playback_position()
		elapsed_label.text = _format_time(seek_slider.value)

func _select_track(index: int) -> void:
	player.stop()
	paused_position = 0.0
	play_button.text = "Play"
	var stream := load(available_paths[index]) as AudioStream
	player.stream = stream
	seek_slider.min_value = 0.0
	seek_slider.max_value = maxf(stream.get_length(), 0.001)
	seek_slider.value = 0.0
	elapsed_label.text = _format_time(0.0)
	duration_label.text = _format_time(stream.get_length())
	_apply_loop(stream, loop_button.button_pressed)
	details_label.text = _describe_stream(stream, available_paths[index])

func _toggle_playback() -> void:
	if player.stream == null:
		return
	if player.playing:
		paused_position = player.get_playback_position()
		player.stop()
		play_button.text = "Play"
	else:
		if paused_position >= player.stream.get_length() - 0.01:
			paused_position = 0.0
		player.play(paused_position)
		play_button.text = "Pause"

func _seek_finished(value_changed: bool) -> void:
	dragging_seek = false
	if not value_changed:
		return
	paused_position = seek_slider.value
	elapsed_label.text = _format_time(paused_position)
	if player.playing:
		player.seek(paused_position)

func _set_loop(enabled: bool) -> void:
	if player.stream != null:
		_apply_loop(player.stream, enabled)

func _apply_loop(stream: AudioStream, enabled: bool) -> void:
	if stream.has_method("set_loop"):
		stream.call("set_loop", enabled)
	elif stream is AudioStreamWAV:
		stream.loop_mode = AudioStreamWAV.LOOP_FORWARD if enabled else AudioStreamWAV.LOOP_DISABLED

func _playback_finished() -> void:
	paused_position = 0.0
	seek_slider.value = 0.0
	elapsed_label.text = _format_time(0.0)
	play_button.text = "Play"

func _describe_stream(stream: AudioStream, path: String) -> String:
	var details := "%s  •  %.1f s" % [stream.get_class(), stream.get_length()]
	if stream.has_method("get_channel_count"):
		details += "  •  %d channel(s)" % stream.call("get_channel_count")
	if stream.has_method("get_sample_rate"):
		details += "  •  %d Hz" % stream.call("get_sample_rate")
	if stream.has_method("get_bits_per_sample"):
		details += "  •  %d-bit" % stream.call("get_bits_per_sample")
	return details + "\n" + path

func _format_time(seconds: float) -> String:
	var whole := maxi(0, floori(seconds))
	return "%d:%02d" % [whole / 60, whole % 60]
