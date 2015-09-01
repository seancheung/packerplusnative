#define DEBUG_INFO 1
#define DEBUG_LOAD 2
#define DEBUG_COMPUTING 4
#define DEBUG_PACKING 8

#include "PackerPlusWrapper.h"
#include "../RBTree/TextureTree.h"
#include <string>
#include <algorithm>
#include "../jsoncpp/json/writer.h"

void concat_path(WCHAR*& dest, const WCHAR* src, int suffix)
{
	const WCHAR* dot = wcschr(src, '.');
	if (!dot || dot == src)
	{
		copy_str(dest, src);
		concat_str(dest, std::to_wstring(suffix).c_str());
	}
	else
	{
		std::wstring str = std::wstring(src);
		int index = str.find_last_of('.');
		std::basic_string<wchar_t> res = str.substr(0, index) + L"_" + std::to_wstring(suffix) + str.substr(index);
		copy_str(dest, res.c_str());
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

void concat_str(WCHAR*& dest, const WCHAR* src)
{
	std::wstring str = std::wstring(dest) + std::wstring(src);
	delete[] dest;
	copy_str(dest, str.c_str());
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

bool pack(const Texture textures[], const int count, Options option, char*& output, const int debug)
{
	if ((debug & DEBUG_INFO) == DEBUG_INFO)
		Debug::log("Checking...");

	/*length check*/
	if (textures == nullptr || count == 0)
	{
		Debug::error("No textures to pack");
		return false;
	}

	if (option.format < CXIMAGE_FORMAT_BMP || option.format > CXIMAGE_FORMAT_WMF)
	{
		Debug::warning("Unsupported format! PNG will be used.");
		option.format = CXIMAGE_FORMAT_PNG;
	}

	/*1, 4, 8, 24*/
	if (option.bit_depth != 1 && option.bit_depth != 4 && option.bit_depth != 8 && option.bit_depth != 24)
	{
		Debug::warning("Color depth invalid! 24 will be used.");
		option.bit_depth = 24;
	}

	if ((debug & DEBUG_INFO) == DEBUG_INFO)
		Debug::log("Loading images...");

	/*load images*/
	std::vector<CxImage*> images;
	for (int i = 0; i < count; i++)
	{
		CxImage* image = new CxImage();
		if (!image->Load(textures[i].path, option.format))
		{
			Debug::error(image->GetLastError());
			delete image;
		}
		else if (image->GetWidth() > option.max_width || image->GetHeight() > option.max_height)
		{
			Debug::error("Texture size is larger than max_size");
			delete image;
			free_vector(images);
			return false;
		}
		else
			images.push_back(image);
	}

	if ((debug & DEBUG_LOAD) == DEBUG_LOAD)
	{
		free_vector(images);
		return false;
	}

	if ((debug & DEBUG_INFO) == DEBUG_INFO)
		Debug::log("Computing rects...");

	/*allocate rects algorithm*/
	switch (option.algorithm)
	{
	case Plain: break;
	case MaxRects:
		{
			std::sort(images.begin(), images.end(), [](const CxImage* a, const CxImage* b)
			          {
				          /*measure weight*/
				          return a->GetWidth() + a->GetHeight() < b->GetWidth() + b->GetHeight();
			          });
		}
		break;
	case TightRects:
		{
			std::sort(images.begin(), images.end(), [](const CxImage* a, const CxImage* b)
			          {
				          /*measure weight*/
				          return a->GetWidth() * a->GetHeight() < b->GetWidth() * b->GetHeight();
			          });
		}
		break;
	default: break;
	}

	int index = count - 1;
	Rect<int> rect = Rect<int>(0, 0, option.max_width, option.max_height);
	std::vector<TextureTree*> trees;
	while (index >= 0)
	{
		trees.push_back(new TextureTree(rect));
		int tindex = 0;

		while (index >= 0)
		{
			bool notfound = false;
			for (size_t i = 0; i < trees.size(); i++)
			{
				if (trees[i]->add_texture(images[index], tindex++, textures[index].name))
					break;
				else if (i == trees.size() - 1)
					notfound = true;
			}
			if (notfound)
			{
				Debug::log("Multiple textures generated");
				break;
			}
			else
				index--;
		}
	}

	if ((debug & DEBUG_COMPUTING) == DEBUG_COMPUTING)
	{
		free_vector(trees);
		return false;
	}

	if ((debug & DEBUG_INFO) == DEBUG_INFO)
		Debug::log("Building Textures...");

	std::vector<Texture*> output_textures;
	std::vector<Sprite*> sprites;

	for (int i = 0; i < trees.size(); i++)
	{
		Texture* texture = new Texture();
		copy_str(texture->name, ("texture_" + std::to_string(i)).c_str());
		if (i > 0)
			concat_path(texture->path, option.output_path, i);
		else
		{
			copy_str(texture->path, option.output_path);
		}

		/*color depth and image format*/
		CxImage image = CxImage(option.max_width, option.max_height, option.bit_depth, option.format);
		if (image.AlphaCreate())
			image.AlphaClear();
		trees[i]->build(image);

		int rw = trees[i]->get_root_width();
		int rh = trees[i]->get_root_height();
		/*apply crop*/
		if (option.crop)
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

		if (image.Save(texture->path, option.format))
			output_textures.push_back(texture);
		else
			Debug::error(image.GetLastError());
	}

	if ((debug & DEBUG_PACKING) == DEBUG_PACKING)
	{
		free_vector(output_textures);
		free_vector(sprites);
		free_vector(trees);
		return false;
	}

	if ((debug & DEBUG_INFO) == DEBUG_INFO)
		Debug::log("Formating json...");

	char* json;
	to_json(output_textures, sprites, json);
	int len = strlen(json) + sizeof(char);
	output = static_cast<char*>(CoTaskMemAlloc(len));
	strcpy_s(output, len, json);
	delete[] json;

	free_vector(output_textures);
	free_vector(sprites);
	free_vector(trees);

	if ((debug & DEBUG_INFO) == DEBUG_INFO)
		Debug::log("Done!");

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
		int len = wcstombs(nullptr, textures[i]->path, 0) + 1;
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
