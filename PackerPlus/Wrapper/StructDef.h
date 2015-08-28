#include "../RBTree/Rect.h"

typedef Rect<float> UVRect;

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

typedef struct Texture
{
	char* path;
	Size size;
} Texture;

typedef struct Sprite
{
	char* name;
	UVRect uv;
	Size size;
	int section;
} Sprite;

typedef struct Atlas
{
	Size maxSize;
	int texture_count;
	Texture* textures;
	int sprite_count;
	Sprite* sprites;
} Atlas;

typedef struct PackData
{
	char* path;
	char* name;
	Size size;
} PackData;
