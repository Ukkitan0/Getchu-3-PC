#!/usr/bin/env python3
"""Anchor-checked Windows portability overlay for snuri00/ps2-web-recomp.

This is applied only to the user's PRIVATE pinned upstream checkout, never to
retail game code. Every transformation is idempotent and rejects changed source
anchors rather than silently producing a partial or misleading build.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path

PINNED_UPSTREAM = '6526f4803d98e10de50bea3af104a4d26e9df033'


def replace_once(source: str, original: str, replacement: str, name: str) -> tuple[str, bool]:
    if original == replacement:
        raise ValueError(f'{name}: replacement is empty')
    count = source.count(original)
    # Some valid compatibility branches preserve POSIX lines inside #else, so
    # the entire already-patched replacement must be checked first.
    if replacement in source:
        if source.count(replacement) == 1 and count <= 1:
            return source, False
        raise ValueError(f'{name}: multiple patched/original anchors detected')
    if count == 1:
        return source.replace(original, replacement, 1), True
    raise ValueError(f'{name}: expected exactly one original anchor, found {count}')


def patch_sources(root: Path, dry_run=False) -> list[dict]:
    changes: dict[str, str] = {}
    tasks: list[dict] = []
    def transform(path: str, anchor: str, replacement: str, label: str):
        p = root / path
        if path not in changes:
            if not p.is_file():
                raise FileNotFoundError(f'Missing pinned upstream source {p}')
            changes[path] = p.read_text(encoding='utf-8')
        changes[path], changed = replace_once(changes[path], anchor, replacement, label)
        tasks.append({'file': path, 'patch': label, 'changed': changed})

    # Shared enum ABI: the GPU class uses the same integral Kind as the host.
    transform('runtime/include/ps2/system.hpp',
              'enum Kind { X11, Wayland, Canvas } kind = X11;',
              'enum Kind { X11, Wayland, Canvas, Windows } kind = X11;',
              'SDL public window target supports Windows')
    transform('runtime/src/gs/gpu/gpu.hpp',
              'enum Kind { X11, Wayland, Canvas } kind;',
              'enum Kind { X11, Wayland, Canvas, Windows } kind;',
              'GPU surface target Windows enum')

    transform('host/window.cpp',
              '#include <sys/resource.h>\n#include <unistd.h>',
              '#if defined(_WIN32)\n#define NOMINMAX\n#include <windows.h>\n#include <psapi.h>\n#else\n#include <sys/resource.h>\n#include <unistd.h>\n#endif',
              'Win32 host APIs instead of POSIX-only headers')
    transform('host/window.cpp',
              '    default: return false;\n    }\n    return ps2::attach_window(target);',
              '#if defined(SDL_VIDEO_DRIVER_WINDOWS)\n'
              '    case SDL_SYSWM_WINDOWS:\n'
              '        target.kind = ps2::WindowTarget::Windows;\n'
              '        target.display = info.info.win.hinstance;\n'
              '        target.window = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(info.info.win.window));\n'
              '        break;\n'
              '#endif\n'
              '    default: return false;\n    }\n    return ps2::attach_window(target);',
              'SDL2 Windows HWND surface attachment')
    transform('host/window.cpp',
              '''double process_cpu_seconds() {
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);
    return static_cast<double>(usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) + static_cast<double>(usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1e6;
}

double resident_mb() {
    long pages = 0, resident = 0;
    if (std::FILE *f = std::fopen("/proc/self/statm", "r")) {
        if (std::fscanf(f, "%ld %ld", &pages, &resident) != 2) resident = 0;
        std::fclose(f);
    }
    return static_cast<double>(resident) * static_cast<double>(sysconf(_SC_PAGESIZE)) / (1024.0 * 1024.0);
}''',
              '''#if defined(_WIN32)
double process_cpu_seconds() {
    FILETIME created{}, exited{}, kernel{}, user{};
    if (!GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user)) return 0.0;
    ULARGE_INTEGER k{}, u{};
    k.LowPart = kernel.dwLowDateTime; k.HighPart = kernel.dwHighDateTime;
    u.LowPart = user.dwLowDateTime; u.HighPart = user.dwHighDateTime;
    return static_cast<double>(k.QuadPart + u.QuadPart) / 10000000.0;
}

double resident_mb() {
    PROCESS_MEMORY_COUNTERS pmc{};
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) return 0.0;
    return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
}
#else
double process_cpu_seconds() {
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);
    return static_cast<double>(usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) + static_cast<double>(usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1e6;
}

double resident_mb() {
    long pages = 0, resident = 0;
    if (std::FILE *f = std::fopen("/proc/self/statm", "r")) {
        if (std::fscanf(f, "%ld %ld", &pages, &resident) != 2) resident = 0;
        std::fclose(f);
    }
    return static_cast<double>(resident) * static_cast<double>(sysconf(_SC_PAGESIZE)) / (1024.0 * 1024.0);
}
#endif''',
              'Win32 CPU and RAM diagnostics')
    transform('runtime/src/gs/gpu/gpu.cpp',
              '''    WGPUSurfaceSourceXlibWindow xlib = WGPU_SURFACE_SOURCE_XLIB_WINDOW_INIT;
    WGPUSurfaceSourceWaylandSurface wayland = WGPU_SURFACE_SOURCE_WAYLAND_SURFACE_INIT;''',
              '''    WGPUSurfaceSourceXlibWindow xlib = WGPU_SURFACE_SOURCE_XLIB_WINDOW_INIT;
    WGPUSurfaceSourceWaylandSurface wayland = WGPU_SURFACE_SOURCE_WAYLAND_SURFACE_INIT;
    WGPUSurfaceSourceWindowsHWND win32 = WGPU_SURFACE_SOURCE_WINDOWS_HWND_INIT;''',
              'Native WGPU Win32 surface descriptor')
    transform('runtime/src/gs/gpu/gpu.cpp',
              '''    } else if (target.kind == SurfaceTarget::Wayland) {
        wayland.display = target.display;
        wayland.surface = target.surface;
        sd.nextInChain = &wayland.chain;
    } else {
        return false;
    }''',
              '''    } else if (target.kind == SurfaceTarget::Wayland) {
        wayland.display = target.display;
        wayland.surface = target.surface;
        sd.nextInChain = &wayland.chain;
    } else if (target.kind == SurfaceTarget::Windows) {
        win32.hinstance = target.display;
        win32.hwnd = reinterpret_cast<void *>(static_cast<std::uintptr_t>(target.window));
        sd.nextInChain = &win32.chain;
    } else {
        return false;
    }''',
              'WebGPU HWND surface selection')
    transform('CMakeLists.txt',
              '''set(PS2_WGPU_DIR "${CMAKE_SOURCE_DIR}/third_party/wgpu-native" CACHE PATH "wgpu-native release folder")
if(EMSCRIPTEN OR EXISTS "${PS2_WGPU_DIR}/lib/libwgpu_native.a")''',
              '''set(PS2_WGPU_DIR "${CMAKE_SOURCE_DIR}/third_party/wgpu-native" CACHE PATH "wgpu-native release folder")
if(WIN32)
    set(PS2_WGPU_LINK "${PS2_WGPU_DIR}/lib/wgpu_native.lib")
else()
    set(PS2_WGPU_LINK "${PS2_WGPU_DIR}/lib/libwgpu_native.a")
endif()
if(EMSCRIPTEN OR EXISTS "${PS2_WGPU_LINK}")''',
              'Windows wgpu-native import library discovery')
    transform('CMakeLists.txt',
              '''        target_include_directories(ps2runtime PRIVATE "${PS2_WGPU_DIR}/include")
        target_link_libraries(ps2runtime PUBLIC "${PS2_WGPU_DIR}/lib/libwgpu_native.a" dl pthread m)
        if(APPLE)''',
              '''        target_include_directories(ps2runtime PRIVATE "${PS2_WGPU_DIR}/include")
        target_link_libraries(ps2runtime PUBLIC "${PS2_WGPU_LINK}")
        if(WIN32)
            # wgpu-native's Rust static library requires Windows system
            # imports in addition to the C GPU interface.
            target_link_libraries(ps2runtime PUBLIC psapi ws2_32 ntdll userenv bcrypt secur32 crypt32 ole32 advapi32 shell32)
        elseif(UNIX)
            target_link_libraries(ps2runtime PUBLIC ${CMAKE_DL_LIBS} pthread m)
        endif()
        if(APPLE)''',
              'Portable CMake platform linking')
    transform('CMakeLists.txt',
              '''    find_package(SDL2 QUIET)
    if(SDL2_FOUND AND NOT EMSCRIPTEN)''',
              '''    find_package(SDL2 QUIET)
    option(AE3_REQUIRE_SDL2 "Fail if no window-capable SDL2 native runner" OFF)
    if(AE3_REQUIRE_SDL2 AND NOT SDL2_FOUND)
        message(FATAL_ERROR "Windows native window build requires SDL2 (use vcpkg sdl2:x64-windows)")
    endif()
    if(SDL2_FOUND AND NOT EMSCRIPTEN)''',
              'Do not silently make headless-only runner')
    transform('CMakeLists.txt',
              '''        target_link_libraries(ps2run PRIVATE SDL2::SDL2)
    endif()''',
              '''        target_link_libraries(ps2run PRIVATE SDL2::SDL2)
        if(WIN32)
            target_link_libraries(ps2run PRIVATE psapi)
        endif()
    endif()''',
              'Link PSAPI in Windows native host')

    # No partial file writes: ALL original anchors checked and transformed first.
    if not dry_run:
        for path, new_text in changes.items():
            p = root / path
            if new_text != p.read_text(encoding='utf-8'):
                p.write_text(new_text, encoding='utf-8', newline='\n')
    return tasks


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--upstream', type=Path, required=True)
    p.add_argument('--dry-run', action='store_true')
    p.add_argument('--report', type=Path)
    a = p.parse_args(argv)
    tasks = patch_sources(a.upstream, dry_run=a.dry_run)
    result = {'pinned_upstream': PINNED_UPSTREAM, 'dry_run': a.dry_run, 'transformations': tasks,
              'files': len({t['file'] for t in tasks}), 'new_changes': sum(t['changed'] for t in tasks)}
    if a.report:
        a.report.parent.mkdir(parents=True, exist_ok=True)
        a.report.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(result, indent=2))
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (OSError, ValueError) as e:
        print('WINDOWS_PATCH_ERROR:', e)
        raise SystemExit(1)