# InFrame

**InFrame: offline group photo organizer**

After a group trip, everyone's photos land in one big shared album, and finding the
ones you're actually in means scrolling through thousands of images. InFrame is a C++
application that uses OpenCV and machine learning to sort a shared album by person,
scenery, and group vs solo shots, so each person can pull out their own photos in
seconds. It runs entirely on your machine, so no photos or face data are ever uploaded.

## Status

🚧 In development.

## Features (planned)

- Find every photo you appear in, using a few reference photos of yourself or by
  picking your face from the people InFrame detects
- Tag photos by person, scenery/places, and group vs solo shots; a photo can carry
  several tags
- Point it at any folder of photos, including subfolders from different phones
- Command-line interface first, with a desktop app to follow
- Runs on Windows

## Building

Prerequisites (Windows):

- Visual Studio Build Tools (with the "Desktop development with C++" workload)
- Git
- CLion or CMake (3.21+) with Ninja
- [vcpkg](https://github.com/microsoft/vcpkg) cloned and bootstrapped, with the
  `VCPKG_ROOT` environment variable pointing to it

Dependencies (OpenCV) are installed automatically by vcpkg on first configure.
Build with the debug preset from a "x64 Native Tools Command Prompt for VS" (or let CLion
load the preset):

```
cmake --preset debug
cmake --build cmake-build-debug
```

## Privacy

- Everything runs locally. No cloud services, no uploads, no network calls with your
  photos or face data.
- This repository never contains personal photos. Tests and examples use licensed
  stock or synthetic images.
- Model files are downloaded during setup rather than stored in the repository.

## License

[MIT](LICENSE)
