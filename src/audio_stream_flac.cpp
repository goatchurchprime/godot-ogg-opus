#include "audio_stream_flac.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {
namespace {

struct InspectionContext {
    const uint8_t *data = nullptr;
    size_t size = 0;
    size_t position = 0;
    int sample_rate = 0;
    int channels = 0;
    int bits_per_sample = 0;
    uint64_t total_samples = 0;
};

FLAC__StreamDecoderReadStatus inspect_read(const FLAC__StreamDecoder *, FLAC__byte buffer[], size_t *bytes, void *client_data) {
    auto *context = static_cast<InspectionContext *>(client_data);
    const size_t available = context->size - context->position;
    const size_t count = std::min(*bytes, available);
    if (count == 0) {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    std::memcpy(buffer, context->data + context->position, count);
    context->position += count;
    *bytes = count;
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderSeekStatus inspect_seek(const FLAC__StreamDecoder *, FLAC__uint64 offset, void *client_data) {
    auto *context = static_cast<InspectionContext *>(client_data);
    if (offset > context->size) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
    context->position = static_cast<size_t>(offset);
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

FLAC__StreamDecoderTellStatus inspect_tell(const FLAC__StreamDecoder *, FLAC__uint64 *offset, void *client_data) {
    *offset = static_cast<InspectionContext *>(client_data)->position;
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus inspect_length(const FLAC__StreamDecoder *, FLAC__uint64 *length, void *client_data) {
    *length = static_cast<InspectionContext *>(client_data)->size;
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

FLAC__bool inspect_eof(const FLAC__StreamDecoder *, void *client_data) {
    const auto *context = static_cast<InspectionContext *>(client_data);
    return context->position >= context->size;
}

FLAC__StreamDecoderWriteStatus inspect_write(const FLAC__StreamDecoder *, const FLAC__Frame *, const FLAC__int32 *const[], void *) {
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void inspect_metadata(const FLAC__StreamDecoder *, const FLAC__StreamMetadata *metadata, void *client_data) {
    if (metadata->type != FLAC__METADATA_TYPE_STREAMINFO) {
        return;
    }
    auto *context = static_cast<InspectionContext *>(client_data);
    context->sample_rate = metadata->data.stream_info.sample_rate;
    context->channels = metadata->data.stream_info.channels;
    context->bits_per_sample = metadata->data.stream_info.bits_per_sample;
    context->total_samples = metadata->data.stream_info.total_samples;
}

void inspect_error(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus, void *) {}

} // namespace

void AudioStreamFLAC::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_data", "data"), &AudioStreamFLAC::set_data);
    ClassDB::bind_method(D_METHOD("get_data"), &AudioStreamFLAC::get_data);
    ClassDB::bind_method(D_METHOD("is_valid"), &AudioStreamFLAC::is_valid);
    ClassDB::bind_method(D_METHOD("get_channel_count"), &AudioStreamFLAC::get_channel_count);
    ClassDB::bind_method(D_METHOD("get_sample_rate"), &AudioStreamFLAC::get_sample_rate);
    ClassDB::bind_method(D_METHOD("get_bits_per_sample"), &AudioStreamFLAC::get_bits_per_sample);
    ClassDB::bind_method(D_METHOD("set_loop", "enabled"), &AudioStreamFLAC::set_loop);
    ClassDB::bind_method(D_METHOD("has_loop"), &AudioStreamFLAC::has_loop);
    ClassDB::bind_method(D_METHOD("set_loop_offset", "seconds"), &AudioStreamFLAC::set_loop_offset);
    ClassDB::bind_method(D_METHOD("get_loop_offset"), &AudioStreamFLAC::get_loop_offset);

    ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_data", "get_data");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop"), "set_loop", "has_loop");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "loop_offset", PROPERTY_HINT_RANGE, "0,36000,0.001,suffix:s"), "set_loop_offset", "get_loop_offset");
}

bool AudioStreamFLAC::inspect_data() {
    length_seconds = 0.0;
    sample_rate = 0;
    channels = 0;
    bits_per_sample = 0;
    if (data.is_empty()) {
        return false;
    }

    InspectionContext context{data.ptr(), static_cast<size_t>(data.size())};
    FLAC__StreamDecoder *decoder = FLAC__stream_decoder_new();
    if (decoder == nullptr) {
        return false;
    }
    const auto status = FLAC__stream_decoder_init_stream(decoder, inspect_read, inspect_seek, inspect_tell,
            inspect_length, inspect_eof, inspect_write, inspect_metadata, inspect_error, &context);
    const bool ok = status == FLAC__STREAM_DECODER_INIT_STATUS_OK &&
            FLAC__stream_decoder_process_until_end_of_metadata(decoder) && context.sample_rate > 0 && context.channels > 0;
    FLAC__stream_decoder_finish(decoder);
    FLAC__stream_decoder_delete(decoder);
    if (!ok) {
        UtilityFunctions::printerr("AudioStreamFLAC: invalid FLAC data");
        return false;
    }
    sample_rate = context.sample_rate;
    channels = context.channels;
    bits_per_sample = context.bits_per_sample;
    length_seconds = static_cast<double>(context.total_samples) / sample_rate;
    return context.total_samples > 0;
}

void AudioStreamFLAC::set_data(const PackedByteArray &p_data) { data = p_data; inspect_data(); emit_changed(); }
PackedByteArray AudioStreamFLAC::get_data() const { return data; }
bool AudioStreamFLAC::is_valid() const { return !data.is_empty() && length_seconds > 0.0 && sample_rate > 0 && channels > 0; }
int AudioStreamFLAC::get_channel_count() const { return channels; }
int AudioStreamFLAC::get_sample_rate() const { return sample_rate; }
int AudioStreamFLAC::get_bits_per_sample() const { return bits_per_sample; }
void AudioStreamFLAC::set_loop(bool p_loop) { loop = p_loop; emit_changed(); }
bool AudioStreamFLAC::has_loop() const { return loop; }
void AudioStreamFLAC::set_loop_offset(double p_seconds) { loop_offset = std::clamp(p_seconds, 0.0, length_seconds); emit_changed(); }
double AudioStreamFLAC::get_loop_offset() const { return loop_offset; }

Ref<AudioStreamPlayback> AudioStreamFLAC::_instantiate_playback() const {
    Ref<AudioStreamPlaybackFLAC> playback;
    playback.instantiate();
    playback->initialize(Ref<AudioStreamFLAC>(const_cast<AudioStreamFLAC *>(this)));
    return playback;
}

String AudioStreamFLAC::_get_stream_name() const { return "FLAC"; }
double AudioStreamFLAC::_get_length() const { return length_seconds; }
bool AudioStreamFLAC::_is_monophonic() const { return false; }

void AudioStreamPlaybackFLAC::_bind_methods() {}
AudioStreamPlaybackFLAC::~AudioStreamPlaybackFLAC() { close_decoder(); }

void AudioStreamPlaybackFLAC::initialize(const Ref<AudioStreamFLAC> &p_stream) {
    stream = p_stream;
    compressed_data = stream->data;
    open_decoder();
}

bool AudioStreamPlaybackFLAC::open_decoder() {
    close_decoder();
    if (stream.is_null() || compressed_data.is_empty()) {
        return false;
    }
    decoder = FLAC__stream_decoder_new();
    if (decoder == nullptr) {
        return false;
    }
    byte_position = 0;
    const auto status = FLAC__stream_decoder_init_stream(decoder, read_callback, seek_callback, tell_callback,
            length_callback, eof_callback, write_callback, metadata_callback, error_callback, this);
    if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK || !FLAC__stream_decoder_process_until_end_of_metadata(decoder)) {
        UtilityFunctions::printerr("AudioStreamPlaybackFLAC: decoder initialization failed");
        close_decoder();
        return false;
    }
    return true;
}

void AudioStreamPlaybackFLAC::close_decoder() {
    if (decoder != nullptr) {
        FLAC__stream_decoder_finish(decoder);
        FLAC__stream_decoder_delete(decoder);
        decoder = nullptr;
    }
    decoded_frames.clear();
    decoded_position = 0;
}

bool AudioStreamPlaybackFLAC::decode_next_block() {
    decoded_frames.clear();
    decoded_position = 0;
    while (decoder != nullptr && decoded_frames.empty()) {
        if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_END_OF_STREAM) {
            return false;
        }
        if (!FLAC__stream_decoder_process_single(decoder)) {
            return false;
        }
    }
    return !decoded_frames.empty();
}

int32_t AudioStreamPlaybackFLAC::_mix_resampled(AudioFrame *p_buffer, int32_t p_frames) {
    if (!playing || decoder == nullptr || p_frames <= 0) {
        return 0;
    }
    int32_t written = 0;
    while (written < p_frames && playing) {
        if (decoded_position >= decoded_frames.size() && !decode_next_block()) {
            if (stream->loop) {
                ++loop_count;
                _seek(stream->loop_offset);
                continue;
            }
            playing = false;
            break;
        }
        const size_t available = decoded_frames.size() - decoded_position;
        const size_t count = std::min<size_t>(available, p_frames - written);
        std::copy_n(decoded_frames.data() + decoded_position, count, p_buffer + written);
        decoded_position += count;
        written += static_cast<int32_t>(count);
        frame_position += count;
    }
    return written;
}

float AudioStreamPlaybackFLAC::_get_stream_sampling_rate() const {
    return stream.is_valid() ? static_cast<float>(stream->sample_rate) : 1.0f;
}

void AudioStreamPlaybackFLAC::_start(double p_from_pos) {
    if (decoder == nullptr && !open_decoder()) {
        playing = false;
        return;
    }
    loop_count = 0;
    begin_resample();
    playing = true;
    _seek(p_from_pos);
}

void AudioStreamPlaybackFLAC::_stop() { playing = false; }
bool AudioStreamPlaybackFLAC::_is_playing() const { return playing; }
int32_t AudioStreamPlaybackFLAC::_get_loop_count() const { return loop_count; }
double AudioStreamPlaybackFLAC::_get_playback_position() const {
    return stream.is_valid() && stream->sample_rate > 0 ? static_cast<double>(frame_position) / stream->sample_rate : 0.0;
}

void AudioStreamPlaybackFLAC::_seek(double p_time) {
    if (decoder == nullptr || stream.is_null() || stream->sample_rate <= 0) {
        return;
    }
    const double seconds = std::clamp(p_time, 0.0, stream->length_seconds);
    const uint64_t target = static_cast<uint64_t>(seconds * stream->sample_rate);
    decoded_frames.clear();
    decoded_position = 0;
    if (FLAC__stream_decoder_seek_absolute(decoder, target)) {
        frame_position = target;
    } else {
        UtilityFunctions::printerr("AudioStreamPlaybackFLAC: seek failed at ", seconds, " seconds");
    }
}

void AudioStreamPlaybackFLAC::_tag_used_streams() {}

FLAC__StreamDecoderReadStatus AudioStreamPlaybackFLAC::read_callback(const FLAC__StreamDecoder *, FLAC__byte buffer[], size_t *bytes, void *client_data) {
    auto *self = static_cast<AudioStreamPlaybackFLAC *>(client_data);
    const size_t size = static_cast<size_t>(self->compressed_data.size());
    const size_t available = size - self->byte_position;
    const size_t count = std::min(*bytes, available);
    if (count == 0) {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    std::memcpy(buffer, self->compressed_data.ptr() + self->byte_position, count);
    self->byte_position += count;
    *bytes = count;
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderSeekStatus AudioStreamPlaybackFLAC::seek_callback(const FLAC__StreamDecoder *, FLAC__uint64 offset, void *client_data) {
    auto *self = static_cast<AudioStreamPlaybackFLAC *>(client_data);
    if (offset > static_cast<FLAC__uint64>(self->compressed_data.size())) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
    self->byte_position = static_cast<size_t>(offset);
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

FLAC__StreamDecoderTellStatus AudioStreamPlaybackFLAC::tell_callback(const FLAC__StreamDecoder *, FLAC__uint64 *offset, void *client_data) {
    *offset = static_cast<AudioStreamPlaybackFLAC *>(client_data)->byte_position;
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus AudioStreamPlaybackFLAC::length_callback(const FLAC__StreamDecoder *, FLAC__uint64 *length, void *client_data) {
    *length = static_cast<AudioStreamPlaybackFLAC *>(client_data)->compressed_data.size();
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

FLAC__bool AudioStreamPlaybackFLAC::eof_callback(const FLAC__StreamDecoder *, void *client_data) {
    const auto *self = static_cast<AudioStreamPlaybackFLAC *>(client_data);
    return self->byte_position >= static_cast<size_t>(self->compressed_data.size());
}

FLAC__StreamDecoderWriteStatus AudioStreamPlaybackFLAC::write_callback(const FLAC__StreamDecoder *, const FLAC__Frame *frame,
        const FLAC__int32 *const buffer[], void *client_data) {
    auto *self = static_cast<AudioStreamPlaybackFLAC *>(client_data);
    const unsigned channels = frame->header.channels;
    const unsigned bits = frame->header.bits_per_sample;
    if (channels == 0 || bits == 0) {
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    const float scale = std::ldexp(1.0f, static_cast<int>(bits) - 1);
    self->decoded_frames.resize(frame->header.blocksize);
    for (unsigned i = 0; i < frame->header.blocksize; ++i) {
        const float left = static_cast<float>(buffer[0][i]) / scale;
        const float right = channels == 1 ? left : static_cast<float>(buffer[1][i]) / scale;
        self->decoded_frames[i] = AudioFrame{left, right};
    }
    self->decoded_position = 0;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void AudioStreamPlaybackFLAC::metadata_callback(const FLAC__StreamDecoder *, const FLAC__StreamMetadata *, void *) {}
void AudioStreamPlaybackFLAC::error_callback(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus status, void *) {
    UtilityFunctions::printerr("AudioStreamPlaybackFLAC: decoder error: ", FLAC__StreamDecoderErrorStatusString[status]);
}

} // namespace godot
