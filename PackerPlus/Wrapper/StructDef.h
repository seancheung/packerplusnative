#include "../RBTree/Rect.h"
#include "../CXImage/ximage.h"

/*float rect data*/
typedef Rect<float> UVRect;
typedef Rect<int> TileRect;
typedef unsigned char BYTE;

/*textures in&out*/
struct Texture
{
	Texture()
	{
		name = nullptr;
		path = nullptr;
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
	Sprite()
	{
		name = nullptr;
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
	Color()
	{
	}

	~Color()
	{
	}

	BYTE r;
	BYTE g;
	BYTE b;
	BYTE a;
};
