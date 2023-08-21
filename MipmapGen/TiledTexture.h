#include<stdint.h>
#include<vector>

enum TiledTextureFormat : uint8_t
{
	Tiled_R8,
	Tiled_R8G8,
	Tiled_R8G8B8A8,
	Tiled_R16,
	Tiled_R16G16B16A16,
	Tield_R32
};

class Tile
{
public:
	Tile(uint8_t* data) { _data = data; }
	~Tile() { delete _data; }
private:
	uint8_t* _data;
};

class TiledTexture
{
public:
	static TiledTexture* Create(void* pixels, TiledTextureFormat format, int width, int height, int tileSize);
	void Save(FILE* f);
	TiledTextureFormat GetFormat() const { return _format; }
	int GetWidth() const { return _width; }
	int GetHeight() const { return _height; }
protected:
	TiledTexture(TiledTextureFormat format, int width, int height, int tileSize, std::vector<Tile>&& tiles);
private:
	int _tileSize;

	int _width;
	int _height;

	TiledTextureFormat _format;
	std::vector<Tile> _tiles;
};