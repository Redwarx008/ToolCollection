#include <cstdint>
#include <cmath>

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

uint32_t* GenerateHeightmap()
{

}

uint16_t* GenerateNormalmap(float* heightmap, int width, int height)
{
	if (!heightmap)
	{
		return nullptr;
	}

	uint32_t resolution = width * height;
	uint16_t* normalmap = new uint16_t[resolution * 2]; //R16G16 texture

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

        uint16_t x = ((normal.x + 1) / 2) * 65535;
        uint16_t y = ((normal.y + 1) / 2) * 65535;

        normalmap[index * 2] = x;
        normalmap[index * 2 + 1] = y;
	}
    return normalmap;
}