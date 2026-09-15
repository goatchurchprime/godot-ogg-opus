#include "resource_format_loader_ogg_opus.h"

#include "audio_stream_ogg_opus.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

void ResourceFormatLoaderOggOpus::_bind_methods() {}

PackedStringArray ResourceFormatLoaderOggOpus::_get_recognized_extensions() const {
    PackedStringArray extensions;
    extensions.push_back("opus");
    return extensions;
}

bool ResourceFormatLoaderOggOpus::_handles_type(const StringName &p_type) const {
    return p_type.is_empty() || p_type == StringName("AudioStream") || p_type == StringName("AudioStreamOggOpus");
}

String ResourceFormatLoaderOggOpus::_get_resource_type(const String &p_path) const {
    return p_path.get_extension().to_lower() == "opus" ? "AudioStreamOggOpus" : "";
}

Variant ResourceFormatLoaderOggOpus::_load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const {
    Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
    if (file.is_null()) {
        UtilityFunctions::printerr("ResourceFormatLoaderOggOpus: cannot open ", p_path);
        return Variant();
    }
    PackedByteArray bytes = file->get_buffer(file->get_length());
    Ref<AudioStreamOggOpus> stream;
    stream.instantiate();
    stream->set_data(bytes);
    if (!stream->is_valid()) {
        UtilityFunctions::printerr("ResourceFormatLoaderOggOpus: not a valid Ogg Opus file: ", p_path);
        return Variant();
    }
    return stream;
}

} // namespace godot
