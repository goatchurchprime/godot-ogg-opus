#include "audio_stream_ogg_opus.h"

#include <algorithm>
#include <cstring>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

void AudioStreamOggOpus::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_data", "data"), &AudioStreamOggOpus::set_data);
    ClassDB::bind_method(D_METHOD("get_data"), &AudioStreamOggOpus::get_data);
    ClassDB::bind_method(D_METHOD("is_valid"), &AudioStreamOggOpus::is_valid);
    ClassDB::bind_method(D_METHOD("get_channel_count"), &AudioStreamOggOpus::get_channel_count);
    ClassDB::bind_method(D_METHOD("set_loop", "enabled"), &AudioStreamOggOpus::set_loop);
    ClassDB::bind_method(D_METHOD("has_loop"), &AudioStreamOggOpus::has_loop);
    ClassDB::bind_method(D_METHOD("set_loop_offset", "seconds"), &AudioStreamOggOpus::set_loop_offset);
    ClassDB::bind_method(D_METHOD("get_loop_offset"), &AudioStreamOggOpus::get_loop_offset);

    ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_data", "get_data");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop"), "set_loop", "has_loop");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "loop_offset", PROPERTY_HINT_RANGE, "0,36000,0.001,suffix:s"), "set_loop_offset", "get_loop_offset");
}

bool AudioStreamOggOpus::inspect_data() {
    length_seconds = 0.0;
    channels = 0;
    if (data.is_empty()) {
        return false;
    }
    int error = 0;
    OggOpusFile *file = op_open_memory(data.ptr(), static_cast<size_t>(data.size()), &error);
    if (file == nullptr) {
        UtilityFunctions::printerr("AudioStreamOggOpus: invalid Ogg Opus data (opusfile error ", error, ")");
        return false;
    }
    const ogg_int64_t samples = op_pcm_total(file, -1);
    channels = op_channel_count(file, -1);
    if (samples >= 0) {
        length_seconds = static_cast<double>(samples) / 48000.0;
    }
    op_free(file);
    return samples >= 0 && channels > 0;
}

void AudioStreamOggOpus::set_data(const PackedByteArray &p_data) {
    data = p_data;
    inspect_data();
    emit_changed();
}

PackedByteArray AudioStreamOggOpus::get_data() const { return data; }
bool AudioStreamOggOpus::is_valid() const { return !data.is_empty() && length_seconds > 0.0 && channels > 0; }
int AudioStreamOggOpus::get_channel_count() const { return channels; }
void AudioStreamOggOpus::set_loop(bool p_loop) { loop = p_loop; emit_changed(); }
bool AudioStreamOggOpus::has_loop() const { return loop; }
void AudioStreamOggOpus::set_loop_offset(double p_seconds) { loop_offset = std::clamp(p_seconds, 0.0, length_seconds); emit_changed(); }
double AudioStreamOggOpus::get_loop_offset() const { return loop_offset; }

Ref<AudioStreamPlayback> AudioStreamOggOpus::_instantiate_playback() const {
    Ref<AudioStreamPlaybackOggOpus> playback;
    playback.instantiate();
    Ref<AudioStreamOggOpus> self(const_cast<AudioStreamOggOpus *>(this));
    playback->initialize(self);
    return playback;
}

String AudioStreamOggOpus::_get_stream_name() const { return "Ogg Opus"; }
double AudioStreamOggOpus::_get_length() const { return length_seconds; }
bool AudioStreamOggOpus::_is_monophonic() const { return false; }

void AudioStreamPlaybackOggOpus::_bind_methods() {}

AudioStreamPlaybackOggOpus::~AudioStreamPlaybackOggOpus() { close_decoder(); }

void AudioStreamPlaybackOggOpus::initialize(const Ref<AudioStreamOggOpus> &p_stream) {
    stream = p_stream;
    compressed_data = stream->data;
    open_decoder();
}

bool AudioStreamPlaybackOggOpus::open_decoder() {
    close_decoder();
    if (stream.is_null() || compressed_data.is_empty()) {
        return false;
    }
    int error = 0;
    decoder = op_open_memory(compressed_data.ptr(), static_cast<size_t>(compressed_data.size()), &error);
    if (decoder == nullptr) {
        UtilityFunctions::printerr("AudioStreamPlaybackOggOpus: opusfile open failed: ", error);
        return false;
    }
    return true;
}

void AudioStreamPlaybackOggOpus::close_decoder() {
    if (decoder != nullptr) {
        op_free(decoder);
        decoder = nullptr;
    }
}

int32_t AudioStreamPlaybackOggOpus::_mix_resampled(AudioFrame *p_buffer, int32_t p_frames) {
    if (!playing || decoder == nullptr || p_frames <= 0) {
        return 0;
    }

    int32_t written = 0;
    while (written < p_frames && playing) {
        const int request = std::min<int32_t>(p_frames - written, 4096);
        const int got = op_read_float_stereo(decoder, decode_buffer, request * 2);
        if (got > 0) {
            for (int i = 0; i < got; ++i) {
                p_buffer[written + i].left = decode_buffer[i * 2];
                p_buffer[written + i].right = decode_buffer[i * 2 + 1];
            }
            written += got;
            continue;
        }
        if (got == OP_HOLE) {
            continue;
        }
        if (got < 0) {
            UtilityFunctions::printerr("AudioStreamPlaybackOggOpus: decode failed: ", got);
            playing = false;
            break;
        }
        if (stream->loop) {
            const ogg_int64_t loop_sample = static_cast<ogg_int64_t>(stream->loop_offset * 48000.0);
            if (op_pcm_seek(decoder, loop_sample) == 0) {
                ++loop_count;
                continue;
            }
        }
        playing = false;
    }
    return written;
}

void AudioStreamPlaybackOggOpus::_start(double p_from_pos) {
    if (decoder == nullptr && !open_decoder()) {
        playing = false;
        return;
    }
    loop_count = 0;
    begin_resample();
    playing = true;
    _seek(p_from_pos);
}

void AudioStreamPlaybackOggOpus::_stop() { playing = false; }
bool AudioStreamPlaybackOggOpus::_is_playing() const { return playing; }
int32_t AudioStreamPlaybackOggOpus::_get_loop_count() const { return loop_count; }

double AudioStreamPlaybackOggOpus::_get_playback_position() const {
    if (decoder == nullptr) {
        return 0.0;
    }
    const ogg_int64_t position = op_pcm_tell(decoder);
    return position < 0 ? 0.0 : static_cast<double>(position) / 48000.0;
}

void AudioStreamPlaybackOggOpus::_seek(double p_time) {
    if (decoder == nullptr) {
        return;
    }
    const double limit = stream.is_valid() ? stream->length_seconds : 0.0;
    const double seconds = std::clamp(p_time, 0.0, limit);
    if (op_pcm_seek(decoder, static_cast<ogg_int64_t>(seconds * 48000.0)) != 0) {
        UtilityFunctions::printerr("AudioStreamPlaybackOggOpus: seek failed at ", seconds, " seconds");
    }
}

void AudioStreamPlaybackOggOpus::_tag_used_streams() {}

} // namespace godot
