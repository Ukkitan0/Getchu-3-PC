#!/usr/bin/env python3
"""Test ONLY the parallel CMake source-registration contract, not game boot."""
import argparse
import shutil
import subprocess
import tempfile
from pathlib import Path

GROUPS = {
    "ASSETS": (
        "include/ae3/original_resource_package.hpp",
        "include/ae3/original_resource_vfi_mount.hpp",
        "include/ae3/original_resource_sz.hpp",
        "src/original_resource_sz.cpp",
        "tests/original_resource_package_test.cpp",
        "tests/original_resource_sz_test.cpp",
        "tests/original_resource_package_retail_test.cpp",
    ),
    "NAVIGATION": (
        "include/ae3/original_nav_stage_route.hpp",
        "include/ae3/original_nav_stage_groups.hpp",
        "src/original_nav_stage_route.cpp",
        "src/original_nav_stage_groups.cpp",
        "tests/original_nav_stage_route_test.cpp",
        "tests/original_nav_stage_groups_test.cpp",
    ),
    "VU0": (
        "include/ae3/original_actor_fallback_vu0_native.hpp",
        "src/original_actor_fallback_vu0_native.cpp",
        "tests/original_actor_fallback_vu0_native_test.cpp",
    ),
    "GRAPHICS": (
        "include/ae3/original_render_instances.hpp",
        "include/ae3/original_render_visibility.hpp",
        "include/ae3/original_render_selector.hpp",
        "src/original_render_instances.cpp",
        "src/original_render_visibility.cpp",
        "src/original_render_selector.cpp",
        "tests/original_render_instances_test.cpp",
        "tests/original_render_visibility_test.cpp",
        "tests/original_render_selector_test.cpp",
    ),
    "INPUT_AUDIO": (
        "include/ae3/original_pad_runtime.hpp",
        "include/ae3/original_audio_runtime.hpp",
        "src/original_pad_runtime.cpp",
        "src/original_pad_win32.cpp",
        "src/original_audio_runtime.cpp",
        "tests/original_input_audio_runtime_test.cpp",
    ),
}

def invoke(*argv, passes=True, expected_text=""):
    proc = subprocess.run(argv, text=True, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, check=False)
    if (proc.returncode == 0) != passes or expected_text not in proc.stdout:
        raise AssertionError(
            f"Unexpected source-registration result: {list(argv)!r}\n"
            f"exit={proc.returncode}, expected_pass={passes}\n"
            f"output={proc.stdout[-4500:]}")
    return proc.stdout

def add_group(root, group):
    for number, relative in enumerate(GROUPS[group]):
        target = root / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        if relative.endswith(".hpp"):
            source = "#pragma once\n"
        elif relative.startswith("tests/"):
            source = "int main(){return 0;}\n"
        else:
            source = f"int cmake_fixture_{group.lower()}_{number}(){{return 0;}}\n"
        target.write_text(source, encoding="utf-8")

def run(manifest, cmake):
    # Disposable CMake-only fixtures. These are not replacement gameplay,
    # game assets, emulation, or a verified full native application build.
    with tempfile.TemporaryDirectory(prefix="ae3_native_cmake_manifest_") as tmp:
        base = Path(tmp)
        root = base / "native"
        root.mkdir()
        (root / "dummy.cpp").write_text("int native_manifest_object(){return 0;}\n")
        (root / "CMakeLists.txt").write_text(
            "cmake_minimum_required(VERSION 3.22)\n"
            "project(NativeManifestContract LANGUAGES CXX)\n"
            "add_library(Getchu3Assets STATIC dummy.cpp)\n"
            "enable_testing()\n"
            f'include("{manifest.as_posix()}")\n', encoding="utf-8")
        build = base / "build"
        invoke(cmake, "-S", str(root), "-B", str(build),
               expected_text="Parallel NAVIGATION: not yet merged")
        invoke(cmake, "-S", str(root), "-B", str(build),
               "-DAE3_REQUIRE_PARALLEL_INTEGRATION=ON", passes=False,
               expected_text="Required native ASSETS workstream")
        (root / "src").mkdir()
        (root / "src/original_nav_stage_route.cpp").write_text(
            "int incomplete_native_stage_source(){return 0;}\n")
        invoke(cmake, "-S", str(root), "-B", str(build),
               "-DAE3_REQUIRE_PARALLEL_INTEGRATION=OFF", passes=False,
               expected_text="Partial parallel NAVIGATION integration")
        for group in GROUPS:
            add_group(root, group)
        invoke(cmake, "-S", str(root), "-B", str(build),
               "-DAE3_REQUIRE_PARALLEL_INTEGRATION=ON",
               expected_text="Parallel INPUT_AUDIO: complete source/test file set")
        invoke(cmake, "--build", str(build), "--parallel", "2")
        output = invoke("ctest", "--test-dir", str(build), "--output-on-failure",
                        expected_text="100% tests passed")
        if "9/9 Test" not in output:
            raise AssertionError("Expected nine native integration test registrations: " + output)
    print("NATIVE_PARALLEL_CMAKE_MANIFEST_PASSED "
          "baseline=1 strict_absent=blocked partial=blocked "
          "complete=5 test_targets=9; build-system fixtures only")

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--cmake", default=shutil.which("cmake") or "cmake")
    options = parser.parse_args()
    if not options.manifest.is_file():
        parser.error("Native specialist source manifest not found")
    run(options.manifest.resolve(), options.cmake)

if __name__ == "__main__":
    main()
