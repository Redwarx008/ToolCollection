#include "TiledTexture.h"
#include <iostream>


void GetPixelInfo(TiledTextureFormat format, int* nchannel, int* depth)
{
	switch (format)
	{
	case Tiled_R8:
		*nchannel = 1;
		*depth = 1;
		break;
	case Tiled_R8G8:
		*nchannel = 2;
		*depth = 1;
		break;
	case Tiled_R8G8B8A8:
		*nchannel = 4;
		*depth = 1;
		break;
	case Tiled_R16:
		*nchannel = 1;
		*depth = 2;
		break;
	case Tiled_R16G16B16A16:
		*nchannel = 4;
		*depth = 2;
		break;
	case Tield_R32:
		*nchannel = 1;
		*depth = 4;
		break;
	default:
		break;
	}
}


TiledTexture::TiledTexture(TiledTextureFormat format, int width, int height, int tileSize, std::vector<Tile>&& tiles)
	:_tiles(tiles)
{
	_format = format;
	_height = height;
	_width = width;
	_tileSize = tileSize;
}

TiledTexture* TiledTexture::Create(void* pixels, TiledTextureFormat format, int width, int height, int tileSize)
{
	TiledTexture* texture = nullptr;

	int nTileX = (width - 1) / tileSize + 1;
	int nTileY = (height - 1) / tileSize + 1;
	std::vector<Tile> tiles(nTileX * nTileY);

	int nChannel;
	int depth;
	GetPixelInfo(format, &nChannel, &depth);
	int pixelSize = nChannel * depth;

	for (int y = 0; y < height; y += tileSize)
	{
		for (int x = 0; x < width; x += tileSize)
		{
			int tileWidth = std::min(tileSize, width - x);
			int tileHeight = std::min(tileSize, height - y);

			uint8_t* tileData = new uint8_t[tileWidth * tileHeight * pixelSize];
			if (tileData == nullptr)
			{
				std::cout << "There is not enough available memory.\n";
				return texture;
			}

			for (int dstY = 0; dstY < tileHeight; ++dstY)
			{
				for (int dstX = 0; dstX < tileWidth; ++dstX)
				{
					int srcX = x + dstX;
					int srcY = y + dstY;
					const uint8_t* src = &static_cast<uint8_t*>(pixels)[(srcX + srcY * width) * pixelSize];
					uint8_t* dst = &tileData[(dstX + dstY * tileWidth) * pixelSize];
					for (int i = 0; i < pixelSize; ++i)
					{
						dst[i] = src[i];
					}
				}
			}
			tiles.push_back(Tile(tileData));
		}
	}

	texture = new TiledTexture(format, width, height, tileSize, std::move(tiles));
	return texture;
}

void TiledTexture::Save(FILE* f)
{

}