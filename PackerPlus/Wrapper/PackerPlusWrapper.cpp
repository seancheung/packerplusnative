#include "PackerPlusWrapper.h"
#include "../RBTree/TextureTree.h"
#include <string>
#include <algorithm>
#include "../jsoncpp/json/writer.h"

void concat_path(const WCHAR* a, int i, WCHAR*& output)
{
	const WCHAR* dot = wcschr(a, '.');
	if (!dot || dot == a)
	{
		const WCHAR* copy = (std::wstring(a) + std::to_wstring(i)).c_str();
		copy_str(output, copy);
	}
	else
	{
		std::wstring str = std::wstring(a);
		int index = str.find_last_of('.');
		const WCHAR* copy = (str.substr(0, index) + _T("_") + std::to_wstring(i) + std::wstring(dot + 1)).c_str();
		copy_str(output, copy);
	}
}

void copy_str(char*& dest, const char* src)
{
	int nlen = strlen(src) + sizeof(char);
	dest = new char[nlen];
	strcpy_s(dest, nlen, src);
}

void copy_str(WCHAR*& dest, const WCHAR* src)
{
	int nlen = wcslen(src) + sizeof(WCHAR);
	dest = new WCHAR[nlen];
	wcscpy_s(dest, nlen, src);
}

void create_empty(const int width, const int height, const WCHAR* path, int bit_depth, int format, const Color color)
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
	image.Clear(0);
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
	image.Save(path, CXIMAGE_FORMAT_PNG);
}

bool pack(const Texture textures[], const int count, const int max_width, const int max_height, const WCHAR* output_path, int bit_depth, int format, char*& output)
{
	/*length check*/
	if (textures == nullptr || count == 0)
	{
		Debug::error("No textures to pack");
		return false;
	}

	if (format < CXIMAGE_FORMAT_BMP || format > CXIMAGE_FORMAT_WMF)
	{
		Debug::warning("Unsupported format! PNG will be used.");
		format = CXIMAGE_FORMAT_PNG;
	}

	/*1, 4, 8, 24*/
	if (bit_depth != 1 && bit_depth != 4 && bit_depth != 8 && bit_depth != 24)
	{
		Debug::warning("Color depth invalid! 24 will be used.");
		bit_depth = 24;
	}

	/*load images*/
	std::vector<CxImage*> images;
	for (int i = 0; i < count; i++)
	{
		CxImage* image = new CxImage();
		if (!image->Load(textures[i].path, format))
		{
			Debug::error(image->GetLastError());
			delete image;
		}
		else if (image->GetWidth() > max_width || image->GetHeight() > max_height)
		{
			Debug::error("Texture size is larger than max_size");
			delete image;
		}
		else
			images.push_back(image);
	}

	/*allocate rects*/

	int index = count - 1;
	Rect<int> rect = Rect<int>(0, 0, max_width, max_height);
	std::vector<TextureTree*> trees;
	while (index >= 0)
	{
		trees.push_back(new TextureTree(rect));
		int tindex = 0;

		while (index >= 0)
		{
			int n = 0;
			for (TextureTree* tree : trees)
			{
				if (tree->add_texture(images[index], tindex++, textures[index].name))
					break;
				if (n = trees.size() - 1)
				{
					Debug::log("Multiple textures generated");
					break;
				}
				n++;
			}
			if (n = trees.size() - 1)
				break;
			index--;
		}
	}

	std::vector<Texture*> output_textures;
	std::vector<Sprite*> sprites;

	for (int i = 0; i < trees.size(); i++)
	{
		Texture* texture = new Texture();
		copy_str(texture->name, ("texture_" + std::to_string(i)).c_str());
		if (i > 0)
			concat_path(output_path, i, texture->path);
		else
		{
			copy_str(texture->path, output_path);
		}

		/*color depth and image format*/
		CxImage image = CxImage(max_width, max_height, bit_depth, format);
		if (image.AlphaCreate())
			image.AlphaSet(0);
		trees[i]->build(image);

		int rw = trees[i]->get_root_width();
		int rh = trees[i]->get_root_height();
		/*apply crop*/
		image.Crop(0, trees[i]->rect.height() - rh, rw, trees[i]->rect.height());
		texture->width = image.GetWidth();
		texture->height = image.GetHeight();
		std::vector<TextureTree*> bounds;
		trees[i]->get_bounds(bounds);
		sort(bounds.begin(), bounds.end());
		for (TextureTree* bound : bounds)
		{
			UVRect uv = UVRect(
				float(bound->rect.xMin) / float(texture->width),
				float(bound->rect.yMin) / float(texture->height),
				float(bound->rect.width()) / float(texture->width),
				float(bound->rect.height()) / float(texture->height));
			Sprite* sprite = new Sprite();
			sprite->rect = bound->rect;
			copy_str(sprite->name, bound->name);
			sprite->uv = uv;
			sprite->section = i;
			sprites.push_back(sprite);
		}

		if (image.Save(texture->path, format))
			output_textures.push_back(texture);
		else
			Debug::error(image.GetLastError());
	}

	char* json;
	to_json(output_textures, sprites, json);
	int len = strlen(json) + sizeof(char);
	output = static_cast<char*>(CoTaskMemAlloc(len));
	strcpy_s(output, len, json);
	delete[] json;

	std::vector<TextureTree*>::iterator tree;
	for (tree = trees.begin(); tree != trees.end(); ++tree)
	{
		delete *tree;
	}
	trees.clear();

	std::vector<Texture*>::iterator texture;
	for (texture = output_textures.begin(); texture != output_textures.end(); ++texture)
	{
		delete *texture;
	}
	sprites.clear();

	std::vector<Sprite*>::iterator sprite;
	for (sprite = sprites.begin(); sprite != sprites.end(); ++sprite)
	{
		delete *sprite;
	}
	sprites.clear();

	return true;
}

void to_json(const std::vector<Texture*> textures, std::vector<Sprite*> sprites, char*& json)
{
	Json::Value output;
	for (int i = 0; i < textures.size(); i++)
	{
		output["textures"][i]["name"] = std::string(textures[i]->name);
		output["textures"][i]["width"] = textures[i]->width;
		output["textures"][i]["height"] = textures[i]->height;
		int len = wcstombs(nullptr, textures[i]->path, 0)+1;
		char* path = new char[len];
		wcstombs(path, textures[i]->path, len);
		//output["textures"][i]["path"] = std::string(reinterpret_cast<const char*>(textures[i]->path), sizeof(WCHAR) / sizeof(char) * wcslen(textures[i]->path));
		output["textures"][i]["path"] = std::string(path);
		delete[] path;
	}

	for (int i = 0; i < sprites.size(); i++)
	{
		output["sprites"][i]["name"] = std::string(sprites[i]->name);
		output["sprites"][i]["section"] = sprites[i]->section;
		output["sprites"][i]["rect"]["xMin"] = sprites[i]->rect.xMin;
		output["sprites"][i]["rect"]["xMax"] = sprites[i]->rect.xMax;
		output["sprites"][i]["rect"]["yMin"] = sprites[i]->rect.yMin;
		output["sprites"][i]["rect"]["yMax"] = sprites[i]->rect.yMax;
		output["sprites"][i]["uv"]["xMin"] = sprites[i]->uv.xMin;
		output["sprites"][i]["uv"]["xMax"] = sprites[i]->uv.xMax;
		output["sprites"][i]["uv"]["yMin"] = sprites[i]->uv.yMin;
		output["sprites"][i]["uv"]["yMax"] = sprites[i]->uv.yMax;
	}

	std::string str = output.toStyledString();
	const char* data = str.c_str();
	copy_str(json, data);
}
