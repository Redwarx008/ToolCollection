#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "inicpp.h"
#include "utility.h"


int main(int argc, char* argv[])
{
    if (std::filesystem::exists("config.ini"))
    {
        ini::IniFile config;
        config.load("config.ini");
        Settings::FileName = config["args"]["FileName"].as<std::string>();
        Settings::MaxMipLevel = config["args"]["MaxMipLevel"].as<int>();
        Settings::ExportPng = config["settings"]["ExportPng"].as<bool>();
        Settings::HeightMultiplier = config["settings"]["HeightMultiplier"].as<int>();
        Settings::GenerateNormalMap = config["settings"]["GenerateNormalmap"].as<bool>();
    }
    else
    {
        if (argc != 3)
        {
            std::cout << "usage: [file] [max mip level].\n";
            return 22;
        }
        Settings::MaxMipLevel = atoi(argv[2]);
        Settings::FileName = argv[1];
    }


    int width, height, nChannel;
    int ok = stbi_info(Settings::FileName.c_str(), &width, &height, &nChannel);
    if (ok == 0)
    {
        std::cout << stbi_failure_reason() << std::endl;
        return 3;
    }

    bool is16Bit = stbi_is_16_bit(Settings::FileName.c_str()) == 1 ? true : false;
    int bitDepth = is16Bit ? 16 : 8;

    void* originPixels;
    if (is16Bit)
    {
        originPixels = stbi_load_16(Settings::FileName.c_str(), &width, &height, &nChannel, 0);
    }
    else
    {
        originPixels = stbi_load(Settings::FileName.c_str(), &width, &height, &nChannel, 0);
    }

    if (Settings::MaxMipLevel < 1 || Settings::MaxMipLevel > std::max(1, (int)std::log2(std::min(width, height))))
    {
        std::cout << "mip level is out of range.\n";
        return 2;
    }

    std::string outputFileName = GetFileNameWithoutSuffix(Settings::FileName);
    if (std::filesystem::exists(outputFileName))
    {
        std::filesystem::remove(outputFileName);
    }
    
    void* inPixels = originPixels;
    for (int mip = 1; mip <= Settings::MaxMipLevel; ++mip)
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

        if (Settings::ExportPng && bitDepth == 8)
        {
            stbi_write_png((outputFileName + '_' + std::to_string(mip) + ".png").c_str(), w, h, nChannel, outPixels, w);
        }
        else
        {
            WriteTexture(outputFileName, outPixels, bitDepth, nChannel, h, w);
        }

        stbi_image_free(inPixels);
        inPixels = outPixels;
        width = w;
        height = h;
    }
    // free the last mip data.
    stbi_image_free(inPixels);
    return 0;
}

