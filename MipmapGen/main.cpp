#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
//#define STBI_ASSERT(x)
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

 static uint16_t get_value_from_pixels(void* pixels, int bitDepth, 
    int x, int y, int width, int nChannel, int channel)
{
    uint16_t value;
    if (bitDepth == 16)
    {
        //uint8_t high = pixels[(x + (size_t)y * width) * 2];
        //uint8_t low = pixels[(x + (size_t)y * width) * 2 + 1];
        //value = (high << 8) | low;

        uint16_t* data = static_cast<uint16_t*>(pixels);
        value = data[(x + (y * width)) * nChannel + channel];
    }
    else
    {
        uint8_t* data = static_cast<uint8_t*>(pixels);
        value = data[(x + (y * width)) * nChannel + channel];
    }
     
    return value;
}

 static void save_value(void* pixels, uint16_t value, int bitDepth, 
    int x, int y, int width, int nChannel, int channel)
{
    if (bitDepth == 16)
    {
        //uint8_t high = (value >> 8) & 0xff;
        //uint8_t low = value & 0xff;
        //pixels->at((x + (size_t)y * width) * 2) = high;
        //pixels->at((x + (size_t)y * width) * 2 + 1) = low;

        uint16_t* data = static_cast<uint16_t*>(pixels);
        data[(x + (y * width)) * nChannel + channel] = value;
    }
    else
    {
        uint8_t* data = static_cast<uint8_t*>(pixels);
        data[(x + (y * width)) * nChannel + channel] = static_cast<uint8_t>((value >> 8) & 0xff);
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

    int width, height, nChannel;
    int ok = stbi_info(fileName.c_str(), &width, &height, &nChannel);
    if (ok == 0)
    {
        std::cout << stbi_failure_reason() << std::endl;
        std::cout << "unsupported image format.\n";
    }

    bool is16Bit = stbi_is_16_bit(fileName.c_str()) == 1 ? true : false;
    int bitDepth = is16Bit ? 16 : 8;

    void* originPixels;
    if (is16Bit)
    {
        originPixels = stbi_load_16(fileName.c_str(), &width, &height, &nChannel, 0);
    }
    else
    {
        originPixels = stbi_load(fileName.c_str(), &width, &height, &nChannel, 0);
    }

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

    void* inPixels = originPixels;
    for (int mip = 1; mip <= maxMipLevel; ++mip)
    {
        int h = floor((float)height / 2);
        int w = floor((float)width / 2);

        void* outPixels = malloc(static_cast<size_t>(h * w * bitDepth / 8) * nChannel);
        if (outPixels == nullptr)
        {
            std::cout << "There is not enough available memory.\n";
            return 0;
        }
        int loop = 0;
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float wx0, wx1, wx2;
                float wy0, wy1, wy2;
                if (width & 1)
                {
                    wx0 = (float)(w - x) / (2 * w + 1);
                    wx1 = (float)w / (2 * w + 1);
                    wx2 = (float)(x + 1) / (2 * w + 1);
                }
                else
                {
                    wx0 = wx1 = 0.5;
                    wx2 = 0;
                }

                if (height & 1)
                {
                    wy0 = (float)(h - y) / (2 * h + 1);
                    wy1 = (float)h / (2 * h + 1);
                    wy2 = (float)(y + 1) / (2 * h + 1);
                }
                else
                {
                    wy0 = wy1 = 0.5;
                    wy2 = 0;
                }

                for (int c = 0; c < nChannel; ++c)
                {
                    float y0 = wx0 * get_value_from_pixels(inPixels, bitDepth, 2 * x, 2 * y, width, nChannel, c) +
                        wx1 * get_value_from_pixels(inPixels, bitDepth, 2 * x + 1, 2 * y, width, nChannel, c) +
                        wx2 * get_value_from_pixels(inPixels, bitDepth, 2 * x + 2, 2 * y, width, nChannel, c);
                    float y1 = wx0 * get_value_from_pixels(inPixels, bitDepth, 2 * x, 2 * y + 1, width, nChannel, c) +
                        wx1 * get_value_from_pixels(inPixels, bitDepth, 2 * x + 1, 2 * y + 1, width, nChannel, c) +
                        wx2 * get_value_from_pixels(inPixels, bitDepth, 2 * x + 2, 2 * y + 1, width, nChannel, c);
                    float y2 = wx0 * get_value_from_pixels(inPixels, bitDepth, 2 * x, 2 * y + 2, width, nChannel, c) +
                        wx1 * get_value_from_pixels(inPixels, bitDepth, 2 * x + 1, 2 * y + 2, width, nChannel, c) +
                        wx2 * get_value_from_pixels(inPixels, bitDepth, 2 * x + 2, 2 * y + 2, width, nChannel, c);

                    float mixValue = wy0 * y0 + wy1 * y1 + wy2 * y2;
                    save_value(&outPixels, mixValue, bitDepth, x, y, w, nChannel, c);
                }
            }
            ++loop;
        }
        stbi_write_png("test.png", w, h, nChannel, outPixels, w * bitDepth / 8);
        stbi_image_free(inPixels);
        inPixels = outPixels;
        width = w;
        height = h;
    }
    // free the last mip data.
    stbi_image_free(inPixels);
}

