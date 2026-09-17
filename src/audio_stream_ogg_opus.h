#pragma once

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_playback_resampled.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <opusfile.h>

namespace godot {

inline constexpr int OPUSFILE_OUTPUT_SAMPLE_RATE = 48000;

class AudioStreamOggOpus;

class AudioStreamPlaybackOggOpus : public AudioStreamPlaybackResampled {
    GDCLASS(AudioStreamPlaybackOggOpus, AudioStreamPlaybackResampled)

    Ref<AudioStreamOggOpus> stream;
    // Retain the exact backing allocation used by op_open_memory even if the
    // resource's data property is replaced while this playback is active.
    PackedByteArray compressed_data;
    OggOpusFile *decoder = nullptr;
    bool playing = false;
    int loop_count = 0;
    float decode_buffer[4096 * 2] = {};

    bool open_decoder();
    void close_decoder();

protected:
    static void _bind_methods();

public:
    AudioStreamPlaybackOggOpus() = default;
    ~AudioStreamPlaybackOggOpus();

    void initialize(const Ref<AudioStreamOggOpus> &p_stream);

    int32_t _mix_resampled(AudioFrame *p_buffer, int32_t p_frames) override;
    float _get_stream_sampling_rate() const override { return static_cast<float>(OPUSFILE_OUTPUT_SAMPLE_RATE); }
    void _start(double p_from_pos = 0.0) override;
    void _stop() override;
    bool _is_playing() const override;
    int32_t _get_loop_count() const override;
    double _get_playback_position() const override;
    void _seek(double p_time) override;
    void _tag_used_streams() override;
};

class AudioStreamOggOpus : public AudioStream {
    GDCLASS(AudioStreamOggOpus, AudioStream)
    friend class AudioStreamPlaybackOggOpus;

    PackedByteArray data;
    double length_seconds = 0.0;
    int channels = 0;
    bool loop = false;
    double loop_offset = 0.0;

    bool inspect_data();

protected:
    static void _bind_methods();

public:
    void set_data(const PackedByteArray &p_data);
    PackedByteArray get_data() const;
    bool is_valid() const;
    int get_channel_count() const;

    void set_loop(bool p_loop);
    bool has_loop() const;
    void set_loop_offset(double p_seconds);
    double get_loop_offset() const;

    Ref<AudioStreamPlayback> _instantiate_playback() const override;
    String _get_stream_name() const override;
    double _get_length() const override;
    bool _is_monophonic() const override;
    double _get_bpm() const override { return 0.0; }
    int32_t _get_beat_count() const override { return 0; }
};

} // namespace godot
