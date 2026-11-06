#include "style.h"

// Keep windows.h small and stop its min/max macros breaking std::max.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

// Off until init_styles finds a console that can show colour.
static bool out_color = false;
static bool err_color = false;

// True if the handle is a console and now understands ANSI codes; false when piped or redirected.
static bool enable_colors(DWORD handle_id) {
    HANDLE handle = GetStdHandle(handle_id);

    DWORD mode = 0;
    if (!GetConsoleMode(handle, &mode)) return false;

    return SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
}

// NO_COLOR set to anything turns colour off (no-color.org); avoids getenv's MSVC warning.
static bool no_color_env() { return GetEnvironmentVariableW(L"NO_COLOR", nullptr, 0) > 0; }

void init_styles(bool no_color) {
    if (no_color || no_color_env()) return;

    out_color = enable_colors(STD_OUTPUT_HANDLE);
    err_color = enable_colors(STD_ERROR_HANDLE);
}

// ANSI code per style; 90 (grey) instead of faint, which the classic console ignores.
static const char* code(Style style) {
    switch (style) {
        case Style::Title:
        case Style::Heading:
            return "\x1b[1m";
        case Style::Match:
            return "\x1b[32m";
        case Style::Warning:
            return "\x1b[33m";
        case Style::Error:
            return "\x1b[31m";
        case Style::Dim:
            return "\x1b[90m";
    }
    return "";
}

std::string styled(Style style, const std::string& text, Stream stream) {
    bool enabled = (stream == Stream::Out) ? out_color : err_color;
    if (!enabled) return text;

    // Reset at the end so the colour never leaks into the next text.
    return code(style) + text + "\x1b[0m";
}
