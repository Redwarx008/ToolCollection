#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
//#define STBI_ASSERT(x)
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const float heightMultiplier = 100;

 stbi_inline static uint16_t get_channel_value(void* pixels, int bitDepth, 
    int x, int y, int width, int height, int nChannel, int channel)
{
    if (x >= width) x = width - 1;
    if (y >= height) y = height - 1;

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

 stbi_inline static void set_channel_value(void* pixels, uint16_t value, int bitDepth,
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
        data[(x + (y * width)) * nChannel + channel] = value;
    }
}

 stbi_inline static std::string GetFileNameWithoutSuffix(const std::string& fileName)
 {
     return fileName.substr(0, fileName.find_last_of('.'));
 }

 static void WriteAsTextureData(const std::string& fileName, void* pixels, int bitDepth, 
     int nChannel, int height, int width)
 {
     FILE* f;
#if defined(_MSC_VER) && _MSC_VER >= 1400
     if (0 != fopen_s(&f, fileName.c_str(), "ab+"))
     {
         return;
     }
#else 
     f = fopen(fileName.c_str(), "ab+");
     if (f == nullptr)
     {
         return;
     }
#endif // (_MSC_VER) && _MSC_VER >= 1400

     uint8_t pixelInfo[] = {(uint8_t)bitDepth, (uint8_t)nChannel};
     uint16_t sizeInfo[] = { (uint16_t)width, (uint16_t)height };
     fwrite(&pixelInfo[0], sizeof(uint8_t), sizeof(pixelInfo) / sizeof(uint8_t), f);
     fwrite(&sizeInfo[0], sizeof(uint16_t), sizeof(sizeInfo) / sizeof(uint16_t), f);

     if (bitDepth == 16)
     {
         uint16_t* data = static_cast<uint16_t*>(pixels);
         for (int y = 0; y < height; ++y)
         {
             for (int x = 0; x < width; ++x)
             {
                 for (int c = 0; c < nChannel; ++c)
                 {
                     uint32_t offset = (x + static_cast<size_t>(width) * y) * nChannel + c;
                     fwrite(&data[offset], sizeof(float), 1, f);
                 }
             }
         }
         fclose(f);
     }
     else
     {
         fwrite(pixels, bitDepth / 8, static_cast<size_t>(width) * height * nChannel, f);
         fclose(f);
     }
 }


int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "usage: [file] [max mip level].\n";
        return 22;
    }
    
    int maxMipLevel = atoi(argv[2]);

    std::string fileName = argv[1];

    int width, height, nChannel;
    int ok = stbi_info(fileName.c_str(), &width, &height, &nChannel);
    if (ok == 0)
    {
        std::cout << stbi_failure_reason() << std::endl;
        return 3;
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
        return 2;
    }

    std::string outputFileName = GetFileNameWithoutSuffix(fileName);
    if (std::filesystem::exists(outputFileName))
    {
        std::filesystem::remove(outputFileName);
    }
    
    void* inPixels = originPixels;
    for (int mip = 1; mip <= maxMipLevel; ++mip)
    {
        int h = floor((float)height / 2);
        int w = floor((float)width / 2);

        void* outPixels = malloc(static_cast<size_t>(h * w * bitDepth / 8) * nChannel);
        if (outPixels == nullptr)
        {
            std::cout << "There is not enough available memory.\n";
            return 12;
        }

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float wx0, wx1, wx2;
                float wy0, wy1, wy2;

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

                for (int c = 0; c < nChannel; ++c)
                {
                    float y0 = wx0 * get_channel_value(inPixels, bitDepth, 2 * x, 2 * y, width, height, nChannel, c) +
                        wx1 * get_channel_value(inPixels, bitDepth, 2 * x + 1, 2 * y, width, height, nChannel, c) +
                        wx2 * get_channel_value(inPixels, bitDepth, 2 * x + 2, 2 * y, width, height, nChannel, c);
                    float y1 = wx0 * get_channel_value(inPixels, bitDepth, 2 * x, 2 * y + 1, width, height, nChannel, c) +
                        wx1 * get_channel_value(inPixels, bitDepth, 2 * x + 1, 2 * y + 1, width, height, nChannel, c) +
                        wx2 * get_channel_value(inPixels, bitDepth, 2 * x + 2, 2 * y + 1, width, height, nChannel, c);
                    float y2 = wx0 * get_channel_value(inPixels, bitDepth, 2 * x, 2 * y + 2, width, height, nChannel, c) +
                        wx1 * get_channel_value(inPixels, bitDepth, 2 * x + 1, 2 * y + 2, width, height, nChannel, c) +
                        wx2 * get_channel_value(inPixels, bitDepth, 2 * x + 2, 2 * y + 2, width, height, nChannel, c);

                    float mixValue = wy0 * y0 + wy1 * y1 + wy2 * y2;
                    set_channel_value(outPixels, mixValue, bitDepth, x, y, w, nChannel, c);
                }
            }
        }
        /*stbi_write_png((GetFileNameWithoutSuffix(fileName) + '_' + std::to_string(mip) + ".png").c_str(), w, h, nChannel, outPixels, w);*/
        WriteAsTextureData(outputFileName, outPixels, bitDepth, nChannel, h, w);
        stbi_image_free(inPixels);
        inPixels = outPixels;
        width = w;
        height = h;
    }
    // free the last mip data.
    stbi_image_free(inPixels);
    return 0;
}

