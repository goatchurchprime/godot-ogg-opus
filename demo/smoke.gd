extends Node

func test_stream(path: String, expected_type: String) -> bool:
	var stream := load(path) as AudioStream
	if stream == null:
		push_error("Could not load " + path)
		return false
	if stream.get_class() != expected_type or not stream.call("is_valid"):
		push_error("Wrong or invalid stream for " + path)
		return false
	if stream.get_length() <= 0.9 or stream.call("get_channel_count") != 1:
		push_error("Unexpected stream metadata for " + path)
		return false
	# Exercise the actual AudioServer mixing path, not only construction/seek.
	var player := AudioStreamPlayer.new()
	add_child(player)
	player.stream = stream
	player.play(0.1)
	await get_tree().create_timer(0.15).timeout
	if player.get_playback_position() <= 0.1:
		push_error("Playback did not advance for " + path)
		return false
	player.seek(0.5)
	await get_tree().create_timer(0.05).timeout
	if player.get_playback_position() < 0.5:
		push_error("Seek failed for " + path)
		return false
	print("XIPH_AUDIO_STREAM_OK type=", expected_type, " length=", stream.get_length(), " channels=", stream.call("get_channel_count"))
	player.stop()
	player.stream = null
	remove_child(player)
	player.free()
	stream = null
	return true

func _ready() -> void:
	if not await test_stream("res://fixture.opus", "AudioStreamOggOpus"):
		get_tree().quit(1)
		return
	if not await test_stream("res://fixture.flac", "AudioStreamFLAC"):
		get_tree().quit(1)
		return
	# Let the AudioServer release its playback reference before extension unload.
	await get_tree().create_timer(0.1).timeout
	print("XIPH_AUDIO_SMOKE_OK")
	get_tree().quit()
