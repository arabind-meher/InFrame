# InFrame

**InFrame: offline group photo organizer**

After a group trip, everyone's photos land in one big shared album, and finding the
ones you're actually in means scrolling through thousands of images. InFrame is a C++
application that uses OpenCV and machine learning to sort a shared album by person,
scenery, and group vs solo shots, so each person can pull out their own photos in
seconds. It runs entirely on your machine, so no photos or face data are ever uploaded.

## Tech stack

- C++20, CMake with Ninja presets, MSVC
- OpenCV 4 with two pretrained models from OpenCV Zoo: YuNet (face detection) and SFace
  (face recognition)
- CLI11 (command line), nlohmann-json (config file)
- vcpkg for all dependencies

## Features

- Find every photo you appear in, using a few reference photos of yourself
- Tag photos as scenery (no faces), solo (one face) or group (two or more faces)
- Point it at any folder of photos, including subfolders from different phones
  (`.jpg`, `.jpeg`, `.png`, `.bmp`)
- Copy your matches into a new folder; the original album is never changed
- Clear terminal output: aligned results table, progress bar and colours
- Runs on Windows, fully offline

## Future plans

- A desktop app on top of the same core
- Pick people from the faces InFrame detects, without reference photos
- Sort copies into folders by person and by tag (scenery, solo, group)
- Faster repeat runs by saving face data for photos already scanned

## Building

Prerequisites (Windows):

- Visual Studio Build Tools (with the "Desktop development with C++" workload)
- Git
- CLion or CMake (3.21+) with Ninja
- [vcpkg](https://github.com/microsoft/vcpkg) cloned and bootstrapped, with the
  `VCPKG_ROOT` environment variable pointing to it

Dependencies (OpenCV, CLI11, nlohmann-json) are installed automatically by vcpkg on first
configure. Build with the debug preset from a "x64 Native Tools Command Prompt for VS" (or
let CLion load the preset):

```
cmake --preset debug
cmake --build cmake-build-debug
```

## Setup

1. Download the two model files into the `models/` folder:
   - `face_detection_yunet_2023mar.onnx` from
     [OpenCV Zoo: YuNet](https://github.com/opencv/opencv_zoo/tree/main/models/face_detection_yunet)
   - `face_recognition_sface_2021dec.onnx` from
     [OpenCV Zoo: SFace](https://github.com/opencv/opencv_zoo/tree/main/models/face_recognition_sface)
2. Copy `config.example.json` to `config.json` in the project root. The defaults work as
   they are; see [Configuration](#configuration) to change them.

## Usage

Find your photos (lists the matches, copies nothing):

```
inframe find --album "D:\Photos\Goa Trip" --refs "D:\Photos\Me"
```

Find and copy them into a new output folder:

```
inframe find --album "D:\Photos\Goa Trip" --refs "D:\Photos\Me" --copy
```

List face counts and tags for every photo in an album:

```
inframe scan --album "D:\Photos\Goa Trip"
```

| Flag | Command | Meaning |
|------|---------|---------|
| `--album <dir>` | `find`, `scan` | Folder of photos to search (required) |
| `--refs <dir>` | `find` | Folder of photos of the person to find, one face each (required) |
| `--copy` | `find` | Copy the matches into a new output folder |
| `-q`, `--quiet` | all | Only print the summary |
| `-v`, `--verbose` | all | Also print non-matches and the settings in use |
| `--no-color` | all | Plain output without colours (setting `NO_COLOR` does the same) |
| `--help` | all | Show help |
| `--version` | all | Show the version |

- Each `--copy` run creates a new folder named after its start time, e.g.
  `output/20261105_140732`, so earlier results are never overwritten.
- Exit codes: `0` success, `1` error (missing folder, bad config, unreadable model),
  `2` wrong usage.

## Configuration

Everything except the album and reference folders comes from `config.json` in the project
root. All keys are required; a missing or invalid value stops the run with an error that
names it.

| Key | Example | Meaning |
|-----|---------|---------|
| `output` | `"output"` | Base folder for copies; each run adds a date-time subfolder |
| `max_dimension` | `1280` | Photos are shrunk to this size before detection (`0` = full size, otherwise 320-4096) |
| `model_directory` | `"models"` | Folder holding the model files |
| `detector_model` | `"face_detection_yunet_2023mar.onnx"` | Face detection model file |
| `recognizer_model` | `"face_recognition_sface_2021dec.onnx"` | Face recognition model file |
| `match_threshold` | `0.363` | Lowest similarity score that counts as a match (0-1) |

Relative paths are relative to the project root, so the tool works from any folder.

## How it works

1. YuNet finds the faces in each photo.
2. SFace turns each face into an embedding, a list of numbers describing it.
3. A photo matches when any of its faces is similar enough to any reference face (cosine
   similarity at or above `match_threshold`).
4. The tag comes from the number of faces: none is scenery, one is solo, more is group.

## Project structure

The code is split into a core library and a thin command-line front end, so a desktop app
can later reuse the core unchanged.

- `src/core/` (`inframe_core` library): no printing and no command-line parsing; data in,
  data out.
  - Directory classes: `AlbumDirectory` and `ReferenceDirectory` (both built on
    `ImageDirectory`) find and load photos, shrunk to `max_dimension`; `OutputDirectory`
    copies matches.
  - Pipeline steps: `FaceDetector` (YuNet), `FaceRecognizer` (SFace), `classify_photo`
    (tagging).
  - `PhotoFinder`: one facade that checks the setup, loads the references and processes
    each photo.
- `src/cli/` (`inframe` executable): parses arguments, reads `config.json`, calls
  `PhotoFinder` and prints the results (table, progress bar, colours).

## Privacy

- Everything runs locally. No cloud services, no uploads, no network calls with your
  photos or face data.
- This repository never contains personal photos.
- Model files are downloaded during setup rather than stored in the repository.
- `config.json` and the `output/` folder are git-ignored, so your paths and copied photos
  stay out of the repository.

## License

[MIT](LICENSE)
