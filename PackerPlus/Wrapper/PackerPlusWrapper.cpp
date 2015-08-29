#include "PackerPlusWrapper.h"
#include "../CXImage/ximage.h"
#include "../RBTree/TextureTree.h"
#include <string>
#include <algorithm>


void convert_char(const char* input, wchar_t*& output)
{
	int len = MultiByteToWideChar(CP_ACP, 0, input, -1, nullptr, 0);
	output = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, input, -1, output, len);
}

void concat(const char* a, const char* b, char*& output)
{
	strcpy(output, (std::string(a) + std::string(b)).c_str());
}

void concat(const char* a, int i, char*& output)
{
	const char* dot = strchr(a, '.');
	if (!dot || dot == a)
		strcpy(output, (std::string(a) + std::to_string(i)).c_str());
	else
	{
		std::string str = std::string(a);
		int index = str.find_last_of('.');
		strcpy(output, (str.substr(0, index) + "_" + std::to_string(i) + std::string(dot + 1)).c_str());
	}
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
	wchar_t* p;
	convert_char(path, p);
	image.Save(p, CXIMAGE_FORMAT_PNG);
	delete[] p;
}

bool pack(const Texture textures[], const int count, const Size max_size, const char* output_path, int bit_depth, int format, int& output_texture_count, Texture*& output_textures, int& output_sprite_count, Sprite*& output_sprites)
{
	/*length check*/
	if (textures == nullptr || count == 0)
	{
		Debug::error("No textures to pack");
		return false;
	}
	/*size check*/
	for (int i = 0; i < count; i++)
	{
		if (textures[i].size.width > max_size.width || textures[i].size.height > max_size.height)
		{
			char* error;
			concat("Target texture size is larger than max_size", textures[i].path, error);
			Debug::error(error);
			delete[] error;
			return false;
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
		wchar_t* p;
		convert_char(textures[i].path, p);
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

	output_texture_count = trees.size();
	/*TODO: call extern delete for managed data*/
	output_textures = new Texture[output_texture_count];

	std::vector<Sprite> sprites;
	for (int i = 0; i < trees.size(); i++)
	{
		strcpy(output_textures[i].name,std::to_string(i).c_str());
		output_textures[i].size = max_size;
		if (i > 0)
			concat(output_path, i, output_textures[i].path);
		else
			strcpy(output_textures[i].path, output_path);
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
		wchar_t* p;
		convert_char(output_textures[i].path, p);
		bool result = image.Save(p, format);
		delete[] p;
		if (result)
		{
			char* output;
			concat("Successfully saved: ", textures[i].path, output);
			Debug::log(output);
			delete[] output;
		}
		else
		{
			char* output;
			concat("Failed to save: ", textures[i].path, output);
			Debug::error(output);
			delete[] output;
		}
	}

	output_sprite_count = sprites.size();
	/*TODO: delete call*/
	output_sprites = new Sprite[output_sprite_count];
	for (int i = 0; i < output_sprite_count; i++)
	{
		output_sprites[i] = sprites[i];
	}

	return true;
}
