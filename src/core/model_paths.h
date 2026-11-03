#pragma once

#include <filesystem>

// Face detector (YuNet) model file.
inline constexpr const char* kDetectorModelFile = "face_detection_yunet_2023mar.onnx";
// Face recognizer (SFace) model file.
inline constexpr const char* kRecognizerModelFile = "face_recognition_sface_2021dec.onnx";

// The models directory next to inframe.exe.
std::filesystem::path model_directory();
