cmake_minimum_required(VERSION 3.21)

project(InFrame VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Core logic, kept separate from the CLI so a future GUI can reuse it.
add_library(
        inframe_core
        STATIC
        src/core/face_detector.cpp
        src/core/face_detector.h
        src/core/face_recognizer.cpp
        src/core/face_recognizer.h
        src/core/photo_tagger.cpp
        src/core/photo_tagger.h
        src/core/image_directory.cpp
        src/core/image_directory.h
        src/core/album_directory.cpp
        src/core/album_directory.h
        src/core/reference_directory.h
        src/core/output_directory.cpp
        src/core/output_directory.h
        src/core/model_paths.cpp
        src/core/model_paths.h
        src/core/photo_results.h
        src/core/photo_finder.cpp
        src/core/photo_finder.h
)

target_include_directories(inframe_core PUBLIC src/core)

target_link_libraries(
        inframe_core
        PUBLIC
        opencv_core
        opencv_objdetect
        PRIVATE
        opencv_imgcodecs
        opencv_imgproc
)

# Command-line program.
add_executable(
        inframe src/cli/main.cpp
        src/cli/console.cpp
        src/cli/console.h
)

target_link_libraries(
        inframe
        PRIVATE
        inframe_core
        CLI11::CLI11
        nlohmann_json::nlohmann_json
)

add_custom_command(
        TARGET inframe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:inframe>/models
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/models/face_detection_yunet_2023mar.onnx $<TARGET_FILE_DIR:inframe>/models
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/models/face_recognition_sface_2021dec.onnx $<TARGET_FILE_DIR:inframe>/models
        COMMENT "Copying Models"
)

# /W4 on project targets only (not as errors).
if (MSVC)
    target_compile_options(inframe_core PRIVATE /W4)
    target_compile_options(inframe PRIVATE /W4)
endif ()
