#!/usr/bin/env python

import os

from SCons.Script import ARGUMENTS, Default, Glob, SConscript

from tools.scons_helpers import build_cmake_dependency, generated_include_path, library_path, output_library


# Keep godot-cpp small and deterministic unless a caller explicitly selects a
# different API profile.
ARGUMENTS.setdefault("build_profile", "build_profile.json")
env = SConscript("godot-cpp/SConstruct")
env = env.Clone()

# Never place generated objects beside source files (some of those sources are
# in submodules). This also keeps source archives and git status clean.
env["SHOBJPREFIX"] = "#obj/"
if os.environ.get("SCONS_CACHE"):
    env.CacheDir(os.environ["SCONS_CACHE"])
    env.Decider("MD5")

output_dir = ARGUMENTS.get("addon_output_dir", "addons/xiph_audio/bin")

env.Append(
    CPPPATH=[
        "src",
        "thirdparty/ogg/include",
        generated_include_path(env, "thirdparty/ogg"),
        "thirdparty/opus/include",
        "thirdparty/opusfile/include",
        "thirdparty/flac/include",
    ],
    # FLAC__NO_DLL prevents Windows headers from marking libFLAC symbols as
    # dllimport when we link the codec statically into the GDExtension.
    CPPDEFINES=["OP_HAVE_LRINTF", "FLAC__NO_DLL"],
    LIBPATH=[
        library_path(env, "thirdparty/opus"),
        library_path(env, "thirdparty/ogg"),
        library_path(env, "thirdparty/flac", "src/libFLAC"),
    ],
    LIBS=["FLAC", "opus", "ogg"],
)

if env["platform"] != "windows":
    env.Append(LIBS=["m"])

sources = Glob("src/*.cpp") + [
    "thirdparty/opusfile/src/info.c",
    "thirdparty/opusfile/src/internal.c",
    "thirdparty/opusfile/src/opusfile.c",
    "thirdparty/opusfile/src/stream.c",
]

library = env.SharedLibrary(
    target=output_library(env, output_dir),
    source=sources,
)
env.NoCache(library)
Default(library)


def build_ogg(target, source, env):
    return build_cmake_dependency(
        env,
        "thirdparty/ogg",
        [
            "-DBUILD_SHARED_LIBS=OFF",
            "-DINSTALL_DOCS=OFF",
            "-DINSTALL_PKG_CONFIG_MODULE=OFF",
            "-DINSTALL_CMAKE_PACKAGE_MODULE=OFF",
        ],
    )


def build_opus(target, source, env):
    options = [
        "-DBUILD_SHARED_LIBS=OFF",
        "-DOPUS_BUILD_SHARED_LIBRARY=OFF",
        "-DOPUS_BUILD_TESTING=OFF",
        "-DOPUS_BUILD_PROGRAMS=OFF",
        "-DOPUS_BUILD_EXAMPLES=OFF",
    ]
    # Opus overrides CMAKE_C_FLAGS with CMAKE_MSVC_RUNTIME_LIBRARY after
    # project configuration. Select its native option so it matches the /MT
    # runtime used by godot-cpp and the opusfile sources.
    if env["platform"] == "windows" and env.get("use_static_cpp", True) and not env.get("debug_crt", False):
        options.append("-DOPUS_STATIC_RUNTIME=ON")

    return build_cmake_dependency(
        env,
        "thirdparty/opus",
        options,
    )


def build_flac(target, source, env):
    options = [
        "-DBUILD_SHARED_LIBS=OFF",
        "-DBUILD_CXXLIBS=OFF",
        "-DBUILD_PROGRAMS=OFF",
        "-DBUILD_EXAMPLES=OFF",
        "-DBUILD_TESTING=OFF",
        "-DBUILD_DOCS=OFF",
        "-DBUILD_UTILS=OFF",
        "-DINSTALL_MANPAGES=OFF",
        "-DINSTALL_PKGCONFIG_MODULES=OFF",
        "-DINSTALL_CMAKE_CONFIG_MODULE=OFF",
        "-DWITH_OGG=OFF",
        "-DENABLE_MULTITHREADING=OFF",
    ]
    # libFLAC 1.5.0 misdetects fseeko on 32-bit Android even though Godot's
    # NDK target is API 24, then aliases the available function to fseek and
    # creates conflicting declarations in the NDK headers.
    if env["platform"] == "android" and env["arch"] in ("arm32", "x86_32"):
        options.append("-DCMAKE_C_FLAGS=-DHAVE_FSEEKO=1")
    if env["platform"] == "windows" and env.get("use_static_cpp", True) and not env.get("debug_crt", False):
        options.append("-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded")
    return build_cmake_dependency(env, "thirdparty/flac", options)


env.Command("build_ogg", [], build_ogg)
env.Command("build_opus", [], build_opus)
env.Command("build_flac", [], build_flac)
