#include "resource_format_loader_flac.h"

#include "audio_stream_flac.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

void ResourceFormatLoaderFLAC::_bind_methods() {}

PackedStringArray ResourceFormatLoaderFLAC::_get_recognized_extensions() const {
    PackedStringArray extensions;
    extensions.push_back("flac");
    return extensions;
}

bool ResourceFormatLoaderFLAC::_handles_type(const StringName &p_type) const {
    return p_type == StringName("AudioStream") || p_type == StringName("AudioStreamFLAC");
}

String ResourceFormatLoaderFLAC::_get_resource_type(const String &p_path) const {
    return p_path.get_extension().to_lower() == "flac" ? "AudioStreamFLAC" : "";
}

bool ResourceFormatLoaderFLAC::_exists(const String &p_path) const { return FileAccess::file_exists(p_path); }

Variant ResourceFormatLoaderFLAC::_load(const String &p_path, const String &, bool, int32_t) const {
    const PackedByteArray bytes = FileAccess::get_file_as_bytes(p_path);
    Ref<AudioStreamFLAC> stream;
    stream.instantiate();
    stream->set_data(bytes);
    if (!stream->is_valid()) {
        UtilityFunctions::printerr("ResourceFormatLoaderFLAC: not a valid FLAC file: ", p_path);
        return Variant();
    }
    return stream;
}

} // namespace godot
