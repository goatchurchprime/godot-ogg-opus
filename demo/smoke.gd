extends Node

func _ready() -> void:
	var stream := load("res://fixture.opus") as AudioStreamOggOpus
	assert(stream != null)
	assert(stream.is_valid())
	assert(stream.get_length() > 0.9)
	assert(stream.get_channel_count() == 1)
	# Exercise the actual AudioServer mixing path, not only construction/seek.
	var player := AudioStreamPlayer.new()
	add_child(player)
	player.stream = stream
	player.play(0.1)
	await get_tree().create_timer(0.15).timeout
	assert(player.get_playback_position() > 0.1)
	player.seek(0.5)
	await get_tree().create_timer(0.05).timeout
	assert(player.get_playback_position() >= 0.5)
	print("GODOT_OGG_OPUS_SMOKE_OK length=", stream.get_length(), " channels=", stream.get_channel_count(), " mixed_position=", player.get_playback_position())
	player.stop()
	player.stream = null
	remove_child(player)
	player.free()
	stream = null
	# Let the AudioServer release its playback reference before extension unload.
	await get_tree().create_timer(0.1).timeout
	get_tree().quit()
