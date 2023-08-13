#include <iostream>

#include "lodepng/lodepng.h"


inline static uint16_t get_value_from_pixels(const std::vector<unsigned char>& pixels, int bitDepth, int x, int y, int width)
{
    uint16_t value;
    if (bitDepth == 16)
    {
        uint8_t high = pixels[(x + (size_t)y * width) * 2];
        uint8_t low = pixels[(x + (size_t)y * width) * 2 + 1];
        value = (high << 8) | low;
    }
    else
    {
        value = pixels[x + (size_t)y * width];
    }
     
    return value;
}

inline static void save_value(std::vector<uint8_t>* pixels, uint16_t value, int bitDepth, int x, int y, int width)
{
    if (bitDepth == 16)
    {
        uint8_t high = (value >> 8) & 0xff;
        uint8_t low = value & 0xff;
        pixels->at((x + (size_t)y * width) * 2) = high;
        pixels->at((x + (size_t)y * width) * 2 + 1) = low;
    }
    else
    {
        pixels->at(x + (size_t)y * width) = (uint8_t)value;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "usage: [file] [max mip level].\n";
    }
    
    int maxMipLevel = atoi(argv[2]);

    std::string fileName = argv[1];
    std::vector<unsigned char> buffer;
    std::vector<unsigned char> pixels;
    if (lodepng::load_file(buffer, fileName) != 0)
    {
        std::cout << "open file failed.\n";
        return 0;
    }

    lodepng::State state;
    uint32_t width;
    uint32_t height;
    unsigned error = lodepng::decode(pixels, width, height, state, buffer);
    if (error) 
    {
        std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
        return 0;
    }
    buffer.clear();

    if (maxMipLevel < 1 || maxMipLevel > std::max(1, (int)std::log2(std::min(width, height))))
    {
        std::cout << "mip level is out of range.\n";
        return 0;
    }

    //  The 16 - bit values are stored in big endian(most
    //  significant byte first) in these arrays.This is the opposite order of the
    //  little endian used by x86 CPU's.
    //if (state.info_png.color.bitdepth == 16)
    //{
    //    for (size_t i = 0; i < pixels.size(); i += 2)
    //    {
    //        std::swap(pixels[i], pixels[i + 1]);
    //    }
    //}

    //if (state.info_png.color.colortype == LCT_GREY)
    //{
    //    unsigned int bitDepth = state.info_png.color.bitdepth;
    //    for (int mip = 1; mip <= maxMipLevel; ++mip)
    //    {
    //        uint32_t h = floor((float)height / 2);
    //        uint32_t w = floor((float)width / 2);
    //        std::vector<unsigned char> mipPixels(h * w * bitDepth / 8);
    //        for (uint32_t y = 0; y < h; ++y)
    //        {
    //            for (uint32_t x = 0; x < w; ++x)
    //            {
    //                float wx0, wx1, wx2;
    //                float wy0, wy1, wy2;
    //                if (width & 1)
    //                {
    //                    wx0 = (float)(w - x) / (2 * w + 1);
    //                    wx1 = (float)w / (2 * w + 1);
    //                    wx2 = (float)(x + 1) / (2 * w + 1);
    //                }
    //                else
    //                {
    //                    wx0 = wx1 = 0.5;
    //                    wx2 = 0;
    //                }

    //                if (height & 1)
    //                {
    //                    wy0 = (float)(h - y) / (2 * h + 1);
    //                    wy1 = (float)h / (2 * h + 1);
    //                    wy2 = (float)(y + 1) / (2 * h + 1);
    //                }
    //                else
    //                {
    //                    wy0 = wy1 = 0.5;
    //                    wy2 = 0;
    //                }

    //                float y0 = wx0 * get_value_from_pixels(pixels, bitDepth, 2 * x, 2 * y, width) + 
    //                           wx1 * get_value_from_pixels(pixels, bitDepth, 2 * x + 1, 2 * y, width) +
    //                           wx2 * get_value_from_pixels(pixels, bitDepth, 2 * x + 2, 2 * y, width);
    //                float y1 = wx0 * get_value_from_pixels(pixels, bitDepth, 2 * x, 2 *y + 1, width) +
    //                           wx1 * get_value_from_pixels(pixels, bitDepth, 2 * x + 1, 2 * y + 1, width) +
    //                           wx2 * get_value_from_pixels(pixels, bitDepth, 2 * x + 2, 2 * y + 1, width);
    //                float y2 = wx0 * get_value_from_pixels(pixels, bitDepth, 2 * x, 2 * y + 2, width) +
    //                           wx1 * get_value_from_pixels(pixels, bitDepth, 2 * x + 1, 2 * y + 2, width) +
    //                           wx2 * get_value_from_pixels(pixels, bitDepth, 2 * x + 2, 2 * y + 2, width);

    //                float mixValue = wy0 * y0 + wy1 * y1 + wy2 * y2;
    //                save_value(&mipPixels, mixValue, bitDepth, x, y, w);
    //            }
    //        }

    //        error = lodepng::encode(std::string(fileName) + "_" + std::to_string(mip) + ".png", mipPixels, w, h, LCT_GREY, bitDepth);
    //        if (error) {
    //            std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    //            return 0;
    //        }
    //    }
    //}
    error = lodepng::encode("test.png", pixels, width, height, LCT_GREY, state.info_png.color.bitdepth);
}

