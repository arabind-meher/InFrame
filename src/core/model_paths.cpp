#include "model_paths.h"

// Keep windows.h small and stop its min/max macros breaking std::max.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

std::filesystem::path model_directory() {
    // Full path of the running program, e.g. E:\InFrame\cmake-build-debug\inframe.exe.
    wchar_t exe_path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exe_path, MAX_PATH);

    // models/ sits next to the exe.
    return std::filesystem::path(exe_path).parent_path() / "models";
}