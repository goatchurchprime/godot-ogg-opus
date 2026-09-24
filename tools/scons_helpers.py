"""Small cross-platform helpers shared by the SCons build and CI."""

import os
import shutil
import subprocess

from SCons.Util import WhereIs


def _build_root(project, env):
    thread_suffix = "_threads" if env["platform"] == "web" and env.get("threads", True) else ""
    return os.path.abspath(os.path.join(project, "build-scons", f"{env['platform']}_{env['arch']}{thread_suffix}"))


def _configuration(env):
    return "RelWithDebInfo" if env.get("dev_build", False) else "Release"


def library_path(env, project):
    return os.path.join(_build_root(project, env), _configuration(env))


def generated_include_path(env, project):
    return os.path.join(_build_root(project, env), "include")


def _cmake_platform_args(env):
    platform = env["platform"]
    arch = env["arch"]
    args = ["-G", "Ninja Multi-Config"]

    if platform == "windows":
        # Follow the compiler SCons/godot-cpp actually discovered. In
        # particular, windows-latest moved from VS 2022 to VS 2026 and a
        # hard-coded generator stopped finding an installed IDE.
        arch_map = {"x86_32": "Win32", "x86_64": "x64", "arm32": "ARM", "arm64": "ARM64"}
        generator_map = {
            "14.5": "Visual Studio 18 2026",
            "14.4": "Visual Studio 17 2022",
            "14.3": "Visual Studio 17 2022",
            "14.2": "Visual Studio 16 2019",
        }
        msvc_version = env.get("MSVC_VERSION", "14.5")
        generator = generator_map.get(msvc_version, "Visual Studio 18 2026")
        return ["-G", generator, "-A", arch_map[arch], "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"]
    if platform == "macos":
        args += ["-DCMAKE_SYSTEM_NAME=Darwin", "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64", "-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15"]
    elif platform == "ios":
        args += ["-DCMAKE_SYSTEM_NAME=iOS", "-DCMAKE_OSX_ARCHITECTURES=arm64", "-DCMAKE_OSX_DEPLOYMENT_TARGET=12.0"]
    elif platform == "android":
        abi = {"arm32": "armeabi-v7a", "arm64": "arm64-v8a", "x86_32": "x86", "x86_64": "x86_64"}[arch]
        ndk = os.environ["ANDROID_NDK_ROOT"]
        args += [
            f"-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake",
            f"-DANDROID_ABI={abi}",
            f"-DANDROID_PLATFORM={env.get('android_api_level', 21)}",
        ]
    elif platform == "web":
        emcc = WhereIs("emcc")
        if not emcc:
            raise RuntimeError("emcc is not on PATH")
        args += [f"-DCMAKE_TOOLCHAIN_FILE={os.path.dirname(emcc)}/cmake/Modules/Platform/Emscripten.cmake"]
        if env.get("threads", True):
            args += ["-DCMAKE_C_FLAGS=-pthread", "-DCMAKE_CXX_FLAGS=-pthread"]

    return args


def build_cmake_dependency(env, project, extra_args):
    build_dir = _build_root(project, env)
    configure = ["cmake", "-S", os.path.abspath(project), "-B", build_dir]
    configure += _cmake_platform_args(env)
    configure += [
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
        "-DCMAKE_C_VISIBILITY_PRESET=hidden",
        "-DCMAKE_ASM_FLAGS=-fPIC",
    ] + extra_args

    try:
        subprocess.run(configure, check=True)
    except subprocess.CalledProcessError:
        # A runner image/toolchain change can make a restored CMake cache stale.
        shutil.rmtree(build_dir, ignore_errors=True)
        subprocess.run(configure, check=True)

    subprocess.run(["cmake", "--build", build_dir, "--config", _configuration(env), "--parallel"], check=True)
    return 0


def output_library(env, output_dir):
    thread_suffix = ".threads" if env["platform"] == "web" and env.get("threads", True) else ""
    filename = f"libogg_opus.{env['platform']}.{env['target']}.{env['arch']}{thread_suffix}{env['SHLIBSUFFIX']}"

    if env["platform"] == "macos":
        framework = os.path.join(output_dir, os.path.splitext(filename)[0] + ".framework")
        resources = os.path.join(framework, "Resources")
        os.makedirs(resources, exist_ok=True)
        plist = """<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
<key>CFBundleExecutable</key><string>{filename}</string>
<key>CFBundleIdentifier</key><string>org.goatchurchprime.ogg-opus</string>
<key>CFBundleInfoDictionaryVersion</key><string>6.0</string>
<key>CFBundleName</key><string>Godot Ogg Opus</string>
<key>CFBundlePackageType</key><string>FMWK</string>
<key>CFBundleSupportedPlatforms</key><array><string>MacOSX</string></array>
<key>LSMinimumSystemVersion</key><string>10.15</string>
</dict></plist>
""".format(filename=filename)
        with open(os.path.join(resources, "Info.plist"), "w", encoding="utf-8") as handle:
            handle.write(plist)
        return os.path.join(framework, filename)

    return os.path.join(output_dir, filename)
