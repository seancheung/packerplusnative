#include "PackerPlusWrapper.h"
#include "../CXImage/ximage.h"
#include "../RBTree/TextureTree.h"
#include <string>
#include <algorithm>


wchar_t* convert_char(const char* input)
{
	int len = MultiByteToWideChar(CP_ACP, 0, input, -1, nullptr, 0);
	wchar_t* output = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, input, -1, output, len);
	return output;
}

char* append(const char* a, const char* b)
{
	return const_cast<char*>((std::string(a) + std::string(b)).c_str());
}

char* append(const char* a, int i)
{
	const char* dot = strchr(a, '.');
	if (!dot || dot == a)
		return const_cast<char*>((std::string(a) + std::to_string(i)).c_str());
	std::string str = std::string(a);
	int index = str.find_last_of('.');
	return const_cast<char*>((str.substr(0, index) + "_" + std::to_string(i) + std::string(dot + 1)).c_str());
}

void pack(const Texture* textures, const int count, const Size max_size, const char* path, Atlas* atlas, int bit_depth, int format)
{
	/*length check*/
	if (textures == nullptr || count == 0)
	{
		Debug::error("No textures to pack");
		return;
	}
	/*size check*/
	for (int i = 0; i < count; i++)
	{
		if (textures[i].size.width > max_size.width || textures[i].size.height > max_size.height)
		{
			Debug::error("Target texture size is larger than max_size");
			Debug::error(textures[i].path);
			return;
		}
	}

	if (format < CXIMAGE_FORMAT_BMP || format > CXIMAGE_FORMAT_WMF)
	{
		Debug::warning("Unsupported format! PNG will be used.");
		format = CXIMAGE_FORMAT_PNG;
	}

	/*1, 4, 8, 24*/
	if (bit_depth != 1 && bit_depth != 4 && bit_depth != 8 && bit_depth != 24)
	{
		Debug::warning("Bitdepth invalid! 24 will be used.");
		bit_depth = 24;
	}

	/*load images*/
	std::vector<CxImage> images;
	for (int i = 0; i < count; i++)
	{
		wchar_t* p = convert_char(textures[i].path);
		CxImage image = CxImage(p, format);
		delete[] p;
		images.push_back(image);
	}

	/*allocate rects*/
	int index = count - 1;
	Rect<int> rect = Rect<int>(0, 0, max_size.width, max_size.height);
	std::vector<TextureTree> trees;
	while (index >= 0)
	{
		trees.push_back(TextureTree(rect));
		int tindex = 0;

		while (index >= 0)
		{
			for (TextureTree tree : trees)
			{
				if (tree.add_texture(images[index], tindex++, textures[index].path))
				{
					Debug::log("Multiple textures generated");
					break;
				}
				index--;
			}
		}
	}

	*atlas = Atlas();
	atlas->maxSize = max_size;
	atlas->texture_count = trees.size();
	atlas->textures = new Texture[atlas->texture_count];

	std::vector<Sprite> sprites;
	for (int i = 0; i < trees.size(); i++)
	{
		atlas->textures[i] = Texture();
		atlas->textures[i].name = const_cast<char*>(std::to_string(i).c_str());
		atlas->textures[i].size = max_size;
		if (i > 0)
			atlas->textures[i].path = append(path, i);
		else
			atlas->textures[i].path = const_cast<char*>(path);
		/*color depth and image format*/
		CxImage image = CxImage(max_size.width, max_size.height, bit_depth, format);
		trees[i].build(image);
		std::vector<TextureTree*> bounds;
		trees[i].get_bounds(bounds);
		sort(bounds.begin(), bounds.end());
		for (TextureTree* bound : bounds)
		{
			UVRect uv = UVRect(
				bound->rect.xMin / trees[i].rect.width(),
				bound->rect.yMin / trees[i].rect.height(),
				bound->rect.width() / trees[i].rect.width(),
				bound->rect.height() / trees[i].rect.height());
			Sprite sprite = Sprite();
			sprite.size = Size(bound->rect.width(), bound->rect.height());
			sprite.name = bound->name;
			sprite.uv = uv;
			sprite.section = i;
			sprites.push_back(sprite);
		}
		wchar_t * p = convert_char(atlas->textures[i].path);
		bool result = image.Save(p, format);
		delete[] p;
		if (result)
			Debug::log(append("Successfully saved: ", textures[i].path));
		else
			Debug::error(append("Failed to save: ", textures[i].path));
	}

	for (int i = 0; i < trees.size(); i++)
	{
		trees[i].dispose_children();
	}
	atlas->sprite_count = sprites.size();
	atlas->sprites = &sprites[0];
}

void create_empty(const int width, const int height, const char* path, int bit_depth, int format, const Color color)
{
	if (format < CXIMAGE_FORMAT_BMP || format > CXIMAGE_FORMAT_WMF)
	{
		Debug::warning("Unsupported format! PNG will be used.");
		format = CXIMAGE_FORMAT_PNG;
	}

	RGBQUAD rgb = RGBQUAD();
	rgb.rgbRed = color.r;
	rgb.rgbGreen = color.g;
	rgb.rgbBlue = color.b;
	rgb.rgbReserved = color.a;
	
	/*1, 4, 8, 24*/
	if (bit_depth != 1 && bit_depth != 4 && bit_depth != 8 && bit_depth != 24)
	{
		Debug::warning("Bitdepth invalid! 24 will be used.");
		bit_depth = 24;
	}

	CxImage image = CxImage(width, height, bit_depth, format);
	bool alpha = false;
	if (rgb.rgbReserved != 255)
		alpha = image.AlphaCreate();
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			image.SetPixelColor(x, y, rgb, alpha);
		}
	}
	wchar_t * p = convert_char(path);
	image.Save(p, CXIMAGE_FORMAT_PNG);
	delete[] p;
}
