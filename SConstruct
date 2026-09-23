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

output_dir = ARGUMENTS.get("addon_output_dir", "addons/ogg_opus/bin")

env.Append(
    CPPPATH=[
        "src",
        "thirdparty/ogg/include",
        generated_include_path(env, "thirdparty/ogg"),
        "thirdparty/opus/include",
        "thirdparty/opusfile/include",
    ],
    CPPDEFINES=["OP_HAVE_LRINTF"],
    LIBPATH=[
        library_path(env, "thirdparty/opus"),
        library_path(env, "thirdparty/ogg"),
    ],
    LIBS=["opus", "ogg"],
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
    return build_cmake_dependency(
        env,
        "thirdparty/opus",
        [
            "-DBUILD_SHARED_LIBS=OFF",
            "-DOPUS_BUILD_SHARED_LIBRARY=OFF",
            "-DOPUS_BUILD_TESTING=OFF",
            "-DOPUS_BUILD_PROGRAMS=OFF",
            "-DOPUS_BUILD_EXAMPLES=OFF",
        ],
    )


env.Command("build_ogg", [], build_ogg)
env.Command("build_opus", [], build_opus)
