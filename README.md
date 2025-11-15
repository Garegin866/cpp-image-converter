# Image converter - CLI Tool (C++)

Small C++ console tool for converting images between simple formats using a minimal image library (**ImgLib**).

## Features

- Supports three formats:
    - **PPM** (plain RGB)
    - **JPEG** (via libjpeg)
    - **BMP** (24-bit BGR, uncompressed)
- Unified interface for formats (`ImageFormatInterface`)
- Uses a common `Image` abstraction inside ImgLib

## Usage

```bash
image_converter INPUT_FILE OUTPUT_FILE