#include <iostream>
#include <string>
#include <string_view>

#include "ppm_image.h"
#include "jpeg_image.h"

using namespace std;
using namespace std::string_view_literals;

enum class Format {
    UNKNOWN,
    PPM,
    JPEG
};

class ImageFormatInterface {
public:
    virtual bool SaveImage(const img_lib::Path& file,
                           const img_lib::Image& image) const = 0;
    virtual img_lib::Image LoadImage(const img_lib::Path& file) const = 0;
    virtual ~ImageFormatInterface() = default;
};

class PpmImageFormat : public ImageFormatInterface {
public:
    bool SaveImage(const img_lib::Path& file,
                   const img_lib::Image& image) const override {
        return img_lib::SavePPM(file, image);
    }

    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadPPM(file);
    }
};

class JpegImageFormat : public ImageFormatInterface {
public:
    bool SaveImage(const img_lib::Path& file,
                   const img_lib::Image& image) const override {
        return img_lib::SaveJPEG(file, image);
    }

    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadJPEG(file);
    }
};

Format GetFormatByExtension(const img_lib::Path& input_file) {
    const string ext = input_file.extension().string();

    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".ppm"sv) {
        return Format::PPM;
    }

    return Format::UNKNOWN;
}

ImageFormatInterface* GetFormatInterface(const img_lib::Path& path) {
    const Format fmt = GetFormatByExtension(path);

    switch (fmt) {
        case Format::PPM: {
            static PpmImageFormat ppm;
            return &ppm;
        }
        case Format::JPEG: {
            static JpegImageFormat jpeg;
            return &jpeg;
        }
        default:
            return nullptr;
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        return 1;
    }

    const img_lib::Path input_path{argv[1]};
    const img_lib::Path output_path{argv[2]};

    ImageFormatInterface* input_fmt = GetFormatInterface(input_path);
    if (!input_fmt) {
        cout << "Unknown format of the input file" << endl;
        return 2;
    }

    ImageFormatInterface* output_fmt = GetFormatInterface(output_path);
    if (!output_fmt) {
        cout << "Unknown format of the output file" << endl;
        return 3;
    }

    img_lib::Image image = input_fmt->LoadImage(input_path);
    if (image.GetWidth() == 0 || image.GetHeight() == 0) {
        return 1;
    }

    if (!output_fmt->SaveImage(output_path, image)) {
        return 1;
    }

    cout << "Successfully converted" << endl;
    return 0;
}
