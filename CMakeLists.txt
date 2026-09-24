cmake_minimum_required(VERSION 3.20)
project(godot_xiph_audio VERSION 0.2.0 LANGUAGES C CXX)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(INSTALL_PKG_CONFIG_MODULE OFF CACHE BOOL "" FORCE)
set(INSTALL_CMAKE_PACKAGE_MODULE OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_SHARED_LIBRARY OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_CXXLIBS OFF CACHE BOOL "" FORCE)
set(BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_UTILS OFF CACHE BOOL "" FORCE)
set(INSTALL_MANPAGES OFF CACHE BOOL "" FORCE)
set(INSTALL_PKGCONFIG_MODULES OFF CACHE BOOL "" FORCE)
set(INSTALL_CMAKE_CONFIG_MODULE OFF CACHE BOOL "" FORCE)
set(WITH_OGG OFF CACHE BOOL "" FORCE)

set(GODOTCPP_BUILD_PROFILE "${CMAKE_CURRENT_SOURCE_DIR}/build_profile.json" CACHE FILEPATH "" FORCE)
add_subdirectory(godot-cpp)
add_subdirectory(thirdparty/ogg EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/opus EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/flac EXCLUDE_FROM_ALL)

# These static archives are linked into the GDExtension shared library.
# Some toolchains enable PIC implicitly; make the requirement explicit so
# clean distribution builds do not depend on that toolchain behaviour.
set_target_properties(ogg opus FLAC PROPERTIES POSITION_INDEPENDENT_CODE ON)

# opusfile's upstream CMake project uses find_package for installed Ogg/Opus.
# Build its file decoder directly here so all three Xiph libraries are linked
# privately into the single GDExtension binary.
add_library(opusfile_static STATIC
    thirdparty/opusfile/src/info.c
    thirdparty/opusfile/src/internal.c
    thirdparty/opusfile/src/opusfile.c
    thirdparty/opusfile/src/stream.c
)
target_include_directories(opusfile_static PUBLIC thirdparty/opusfile/include)
target_compile_definitions(opusfile_static PRIVATE OP_HAVE_LRINTF)
target_link_libraries(opusfile_static PUBLIC Ogg::ogg Opus::opus)
if(NOT WIN32)
    target_link_libraries(opusfile_static PUBLIC m)
endif()
set_target_properties(opusfile_static PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    C_STANDARD 99
)

add_library(godot_xiph_audio SHARED
    src/audio_stream_ogg_opus.cpp
    src/audio_stream_flac.cpp
    src/resource_format_loader_ogg_opus.cpp
    src/resource_format_loader_flac.cpp
    src/register_types.cpp
)
target_include_directories(godot_xiph_audio PRIVATE src thirdparty/flac/include)
target_link_libraries(godot_xiph_audio PRIVATE godot-cpp opusfile_static FLAC)
target_compile_features(godot_xiph_audio PRIVATE cxx_std_17)
set_target_properties(godot_xiph_audio PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    POSITION_INDEPENDENT_CODE ON
    BUILD_RPATH_USE_ORIGIN ON
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/addons/xiph_audio/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/addons/xiph_audio/bin"
)
get_target_property(GODOTCPP_SUFFIX godot-cpp GODOTCPP_SUFFIX)
set_target_properties(godot_xiph_audio PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "xiph_audio${GODOTCPP_SUFFIX}"
)
