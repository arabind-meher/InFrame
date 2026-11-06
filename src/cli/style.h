#pragma once

// Terminal colours; console.cpp decides what gets which style.

#include <string>

// Styles by meaning, so the colour choice lives only in style.cpp.
enum class Style { Title, Heading, Match, Warning, Error, Dim };

// stdout and stderr are checked apart (e.g. stdout redirected, stderr still on screen).
enum class Stream { Out, Err };

// Turns colour on for each stream that is a real console; off for all with no_color or NO_COLOR.
void init_styles(bool no_color);

// Console shows UTF-8 (symbols, any file name); the old code page is restored at exit.
void init_utf8();

// The text wrapped in colour codes, or unchanged when colour is off for that stream.
std::string styled(Style style, const std::string& text, Stream stream = Stream::Out);
