#include <cstdint>
#include <cmath>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
//#define STBI_ASSERT(x)
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct Vector3
{
    float x, y, z;
    Vector3(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    Vector3 Normalized()
    {
        Vector3 v = *this;
        double squared = v.x * v.x + v.y * v.y + v.z * v.z;
        if (squared == 0)
        {
            v.x = v.y = v.z = 0;
        }
        else
        {
            double length = sqrt(squared);
            v.x /= length;
            v.y /= length;
            v.z /= length;
        }
        return v;
    }
};

struct Settings
{
    static std::string FileName;

    static int MaxMipLevel;

    static float HeightMultiplier;

    static bool GenerateNormalMap;

    static bool ExportPng;
};

stbi_inline uint16_t get_channel_value(void* pixels, int bitDepth,
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

stbi_inline void set_channel_value(void* pixels, uint16_t value, int bitDepth,
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


stbi_inline std::string GetFileNameWithoutSuffix(const std::string& fileName)
{
    return fileName.substr(0, fileName.find_last_of('.'));
}

void WriteTexture(const std::string& fileName, void* pixels, int bitDepth,
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

    uint8_t pixelInfo[] = { (uint8_t)bitDepth, (uint8_t)nChannel };
    uint16_t sizeInfo[] = { (uint16_t)width, (uint16_t)height };
    fwrite(&pixelInfo[0], sizeof(uint8_t), sizeof(pixelInfo) / sizeof(uint8_t), f);
    fwrite(&sizeInfo[0], sizeof(uint16_t), sizeof(sizeInfo) / sizeof(uint16_t), f);

    if (bitDepth == 16)
    {
        uint16_t* data = static_cast<uint16_t*>(pixels);
        // for heightmap
        if (nChannel == 1)
        {
            ExportHeightmap(f, data, width, height);
        }
        else
        {
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
        }

        fclose(f);
    }
    else
    {
        fwrite(pixels, bitDepth / 8, static_cast<size_t>(width) * height * nChannel, f);
        fclose(f);
    }
}

void ExportHeightmap(FILE* f, uint16_t* source, int width, int height)
{
    
}

uint8_t* GenerateNormalmap(float* heightmap, int width, int height)
{
	if (!heightmap)
	{
		return nullptr;
	}

	uint32_t resolution = width * height;
    uint8_t* normalmap = new uint8_t[resolution * 2]; //R16G16 texture

	for (int index = 0; index < resolution; ++index)
	{
        int upIndex = index - width < 0 ? index : index - width;
        int downIndex = index + width > resolution - 1 ? index : index + width;
        int leftIndex = index - 1 < 0 ? index : index - 1;
        int rightIndex = index + 1 > resolution - 1 ? index : index + 1;
        float up = heightmap[upIndex];
        float down = heightmap[downIndex];
        float left = heightmap[leftIndex];
        float right = heightmap[rightIndex];

        Vector3 normal = { left - right, up - down, 2.0f };
        normal = normal.Normalized();

        uint8_t x = ((normal.x + 1) / 2) * 255;
        uint8_t y = ((normal.y + 1) / 2) * 255;

        normalmap[index * 2] = x;
        normalmap[index * 2 + 1] = y;
	}
    return normalmap;
}