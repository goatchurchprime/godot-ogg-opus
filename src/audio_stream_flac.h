#pragma once

#include <vector>

#include <FLAC/stream_decoder.h>

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_playback_resampled.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

namespace godot {

class AudioStreamFLAC;

class AudioStreamPlaybackFLAC : public AudioStreamPlaybackResampled {
    GDCLASS(AudioStreamPlaybackFLAC, AudioStreamPlaybackResampled)

    Ref<AudioStreamFLAC> stream;
    PackedByteArray compressed_data;
    FLAC__StreamDecoder *decoder = nullptr;
    size_t byte_position = 0;
    std::vector<AudioFrame> decoded_frames;
    size_t decoded_position = 0;
    uint64_t frame_position = 0;
    bool playing = false;
    int loop_count = 0;

    bool open_decoder();
    void close_decoder();
    bool decode_next_block();

    static FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *, FLAC__byte[], size_t *, void *);
    static FLAC__StreamDecoderSeekStatus seek_callback(const FLAC__StreamDecoder *, FLAC__uint64, void *);
    static FLAC__StreamDecoderTellStatus tell_callback(const FLAC__StreamDecoder *, FLAC__uint64 *, void *);
    static FLAC__StreamDecoderLengthStatus length_callback(const FLAC__StreamDecoder *, FLAC__uint64 *, void *);
    static FLAC__bool eof_callback(const FLAC__StreamDecoder *, void *);
    static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *, const FLAC__Frame *, const FLAC__int32 *const[], void *);
    static void metadata_callback(const FLAC__StreamDecoder *, const FLAC__StreamMetadata *, void *);
    static void error_callback(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus, void *);

protected:
    static void _bind_methods();

public:
    AudioStreamPlaybackFLAC() = default;
    ~AudioStreamPlaybackFLAC();

    void initialize(const Ref<AudioStreamFLAC> &p_stream);
    int32_t _mix_resampled(AudioFrame *p_buffer, int32_t p_frames) override;
    float _get_stream_sampling_rate() const override;
    void _start(double p_from_pos = 0.0) override;
    void _stop() override;
    bool _is_playing() const override;
    int32_t _get_loop_count() const override;
    double _get_playback_position() const override;
    void _seek(double p_time) override;
    void _tag_used_streams() override;
};

class AudioStreamFLAC : public AudioStream {
    GDCLASS(AudioStreamFLAC, AudioStream)
    friend class AudioStreamPlaybackFLAC;

    PackedByteArray data;
    double length_seconds = 0.0;
    int sample_rate = 0;
    int channels = 0;
    int bits_per_sample = 0;
    bool valid = false;
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
    int get_sample_rate() const;
    int get_bits_per_sample() const;

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
