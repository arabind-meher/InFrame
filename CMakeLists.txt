cmake_minimum_required(VERSION 3.21)

project(InFrame VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV CONFIG REQUIRED)

# Core logic, kept separate from the CLI so a future GUI can reuse it.
add_library(inframe_core STATIC src/core/core.cpp)
target_include_directories(inframe_core PUBLIC src/core)
target_link_libraries(inframe_core PUBLIC opencv_core PRIVATE opencv_imgcodecs)

# Command-line program.
add_executable(inframe src/cli/main.cpp)
target_link_libraries(inframe PRIVATE inframe_core)

# /W4 on project targets only (not as errors).
if(MSVC)
    target_compile_options(inframe_core PRIVATE /W4)
    target_compile_options(inframe PRIVATE /W4)
endif()
