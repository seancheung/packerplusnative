#include "../RBTree/Rect.h"

/*float rect data*/
typedef Rect<float> UVRect;
typedef unsigned char BYTE;

/*size info*/
typedef union Size
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
} Size;

/*textures in&out*/
typedef struct Texture
{
	char* path;
	char* name;
	Size size;
} Texture;

/*sprite data out*/
typedef struct Sprite
{
	char* name;
	UVRect uv;
	Size size;
	int section;
} Sprite;

/*atlas data out*/
typedef struct Atlas
{
	Size maxSize;
	int texture_count;
	Texture* textures;
	int sprite_count;
	Sprite* sprites;
} Atlas;

/*RGB*/
typedef struct Color
{
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE a;
} Color;
