#include "bmp_image.h"
#include "pack_defines.h"

#include <cstdint>
#include <fstream>
#include <vector>
#include <algorithm>

namespace img_lib {

    PACKED_STRUCT_BEGIN BitmapFileHeader {
        uint16_t bfType;
        uint32_t bfSize;
        uint32_t bfReserved;
        uint32_t bfOffBits;
    }
    PACKED_STRUCT_END

    PACKED_STRUCT_BEGIN BitmapInfoHeader {
        uint32_t biSize;
        int32_t  biWidth;
        int32_t  biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        int32_t  biXPelsPerMeter;
        int32_t  biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t biClrImportant;
    }
    PACKED_STRUCT_END

    static int GetBMPStride(int w) {
        return 4 * ((w * 3 + 3) / 4);
    }

    Image LoadBMP(const Path& file) {
        std::ifstream in(file, std::ios::binary);
        if (!in) {
            return {};
        }

        BitmapFileHeader file_header;
        BitmapInfoHeader info_header;

        in.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
        in.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));
        if (!in) {
            return {};
        }

        if (file_header.bfType != 0x4D42) {
            return {};
        }
        if (info_header.biPlanes != 1 || info_header.biBitCount != 24 || info_header.biCompression != 0) {
            return {};
        }

        int width = info_header.biWidth;
        int height = info_header.biHeight;
        if (width <= 0 || height == 0) {
            return {};
        }

        int abs_height = height > 0 ? height : -height;
        int stride = GetBMPStride(width);

        Image image(width, abs_height, Color::Black());

        in.seekg(static_cast<std::streamoff>(file_header.bfOffBits), std::ios::beg);
        if (!in) {
            return {};
        }

        std::vector<std::uint8_t> row(static_cast<size_t>(stride));

        for (int y = 0; y < abs_height; ++y) {
            int dst_y = (height > 0) ? (abs_height - 1 - y) : y;
            in.read(reinterpret_cast<char*>(row.data()), stride);
            if (!in) {
                return {};
            }

            Color* dst_line = image.GetLine(dst_y);
            for (int x = 0; x < width; ++x) {
                size_t idx = static_cast<size_t>(x) * 3;
                std::uint8_t b = row[idx + 0];
                std::uint8_t g = row[idx + 1];
                std::uint8_t r = row[idx + 2];
                dst_line[x] = Color{std::byte{r}, std::byte{g}, std::byte{b}, std::byte{255}};
            }
        }

        return image;
    }

    bool SaveBMP(const Path& file, const Image& image) {
        int width = image.GetWidth();
        int height = image.GetHeight();
        if (width <= 0 || height <= 0) {
            return false;
        }

        int stride = GetBMPStride(width);
        std::uint32_t data_size = static_cast<std::uint32_t>(stride) * static_cast<std::uint32_t>(height);
        std::uint32_t file_header_size = sizeof(BitmapFileHeader);
        std::uint32_t info_header_size = sizeof(BitmapInfoHeader);
        std::uint32_t data_offset = file_header_size + info_header_size;
        std::uint32_t file_size = data_offset + data_size;

        BitmapFileHeader file_header;
        file_header.bfType = 0x4D42;
        file_header.bfSize = file_size;
        file_header.bfReserved = 0;
        file_header.bfOffBits = data_offset;

        BitmapInfoHeader info_header;
        info_header.biSize = info_header_size;
        info_header.biWidth = width;
        info_header.biHeight = height;
        info_header.biPlanes = 1;
        info_header.biBitCount = 24;
        info_header.biCompression = 0;
        info_header.biSizeImage = data_size;
        info_header.biXPelsPerMeter = 11811;
        info_header.biYPelsPerMeter = 11811;
        info_header.biClrUsed = 0;
        info_header.biClrImportant = 0x1000000;

        std::ofstream out(file, std::ios::binary);
        if (!out) {
            return false;
        }

        out.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
        out.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));
        if (!out) {
            return false;
        }

        std::vector<std::uint8_t> row(static_cast<size_t>(stride));

        for (int y = 0; y < height; ++y) {
            const Color* src_line = image.GetLine(height - 1 - y);
            for (int x = 0; x < width; ++x) {
                size_t idx = static_cast<size_t>(x) * 3;
                const Color& c = src_line[x];
                row[idx + 0] = static_cast<std::uint8_t>(c.b);
                row[idx + 1] = static_cast<std::uint8_t>(c.g);
                row[idx + 2] = static_cast<std::uint8_t>(c.r);
            }
            std::fill(row.begin() + width * 3, row.end(), 0);
            out.write(reinterpret_cast<const char*>(row.data()), stride);
            if (!out) {
                return false;
            }
        }

        return true;
    }

}  // namespace img_lib
