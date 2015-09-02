#include "../RBTree/Rect.h"
#include "../CXImage/ximage.h"

/*float rect data*/
typedef Rect<float> UVRect;
typedef Rect<int> TileRect;
typedef unsigned char BYTE;

/*textures in&out*/
struct Texture
{
	Texture() :path(nullptr), name(nullptr)
	{
	}

	~Texture()
	{
		if (name != nullptr)
			delete[] name;
		if (path != nullptr)
			delete[] path;
	}

	WCHAR* path;
	char* name;
	int width;
	int height;
};

/*sprite data out*/
struct Sprite
{
	Sprite() :name(nullptr)
	{
	}

	~Sprite()
	{
		if (name != nullptr)
			delete[] name;
	}

	char* name;
	UVRect uv;
	TileRect rect;
	int section;
};

/*RGB*/
struct Color
{
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE a;
};

/*packing algorithm*/
enum PackingAlgorithm
{
	/*no sorting*/
	Plain,
	/*sorting by size ascending(w+h)*/
	MaxRects,
	/*sorting by size ascending(w*h)*/
	TightRects,
};

/*packing options*/
struct Options
{
	const int max_width;
	const int max_height;
	const WCHAR* output_path;
	int bit_depth;
	int format;
	const bool crop;
	const PackingAlgorithm algorithm;
};

/*atlas data*/
struct Atlas
{
	Atlas() :textures(nullptr), sprites(nullptr)
	{
	}

	~Atlas()
	{
		if (textures != nullptr)
			delete[] textures;
		if (sprites != nullptr)
			delete[] sprites;
	}

	Texture** textures;
	Sprite** sprites;
};
