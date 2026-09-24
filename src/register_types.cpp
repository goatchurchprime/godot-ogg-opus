#include "register_types.h"

#include "audio_stream_flac.h"
#include "audio_stream_ogg_opus.h"
#include "resource_format_loader_flac.h"
#include "resource_format_loader_ogg_opus.h"

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

static Ref<ResourceFormatLoaderOggOpus> opus_resource_loader;
static Ref<ResourceFormatLoaderFLAC> flac_resource_loader;

void initialize_xiph_audio_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    ClassDB::register_class<AudioStreamOggOpus>();
    ClassDB::register_class<AudioStreamPlaybackOggOpus>();
    ClassDB::register_class<ResourceFormatLoaderOggOpus>();
    ClassDB::register_class<AudioStreamFLAC>();
    ClassDB::register_class<AudioStreamPlaybackFLAC>();
    ClassDB::register_class<ResourceFormatLoaderFLAC>();
    opus_resource_loader.instantiate();
    flac_resource_loader.instantiate();
    ResourceLoader::get_singleton()->add_resource_format_loader(opus_resource_loader, true);
    ResourceLoader::get_singleton()->add_resource_format_loader(flac_resource_loader, true);
}

void uninitialize_xiph_audio_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    if (opus_resource_loader.is_valid()) {
        ResourceLoader::get_singleton()->remove_resource_format_loader(opus_resource_loader);
        opus_resource_loader.unref();
    }
    if (flac_resource_loader.is_valid()) {
        ResourceLoader::get_singleton()->remove_resource_format_loader(flac_resource_loader);
        flac_resource_loader.unref();
    }
}

extern "C" {
GDExtensionBool GDE_EXPORT xiph_audio_library_init(
        GDExtensionInterfaceGetProcAddress p_get_proc_address,
        GDExtensionClassLibraryPtr p_library,
        GDExtensionInitialization *r_initialization) {
    GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
    init_obj.register_initializer(initialize_xiph_audio_module);
    init_obj.register_terminator(uninitialize_xiph_audio_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
    return init_obj.init();
}
}
