#include "../RBTree/Rect.h"

/*float rect data*/
typedef Rect<float> UVRect;
typedef unsigned char BYTE;

/*size info*/
union Size
{
	int width;
	int height;

	explicit Size(int width, int height)
		: width(width),height(height)
	{
	}

	Size()
	{
	}

	~Size()
	{
	}
};

/*textures in&out*/
struct Texture
{
	Texture()
	{
	}

	~Texture()
	{
		delete[] name;
		delete[] path;
	}

	char* path;
	char* name;
	Size size;
};

/*sprite data out*/
struct Sprite
{
	Sprite()
	{
	}

	~Sprite()
	{
		delete[] name;
	}

	char* name;
	UVRect uv;
	Size size;
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
