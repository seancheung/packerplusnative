#define DEBUG_INFO 1
#define DEBUG_LOAD 2
#define DEBUG_COMPUTING 4
#define DEBUG_PACKING 8
#define DEBUG_JSON 16

#include "PackerPlusWrapper.h"
#include "../RBTree/TextureTree.h"
#include <string>
#include <algorithm>
#include "assert.h"
#include "../jsoncpp/json/writer.h"
#include <fstream>

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

bool pack(const Texture textures[], const int count, Options option, char*& json_data, const int debug)
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
				break;
			}
			else
				index--;
		}
	}

	if (trees.size() > 1)
		Debug::log("Multiple textures generated");

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
		if (trees.size() > 1)
			concat_path(texture->path, option.output_path, i);
		else
			copy_str(texture->path, option.output_path);

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
	to_json(output_textures, sprites, json, debug);

	if ((debug & DEBUG_JSON) == DEBUG_JSON)
	{
		delete[] json;
		free_vector(output_textures);
		free_vector(sprites);
		free_vector(trees);
		return false;
	}


	/*std::wstring path(option.output_path);
	int lastindex = path.find_last_of(L".");
	std::wstring jpath = path.substr(0, lastindex) + L".json";
	std::ofstream out(jpath);
	if (out)
	{
		out << json;
		out.close();
	}
	else
	{
		Debug::error("Failed to write data");
		delete[] json;
		return false;
	}*/

	unsigned long long size = strlen(json) + sizeof(char);
	json_data = static_cast<char*>(CoTaskMemAlloc(size));
	strcpy_s(json_data, size, json);

	delete[] json;
	free_vector(output_textures);
	free_vector(sprites);
	free_vector(trees);

	if ((debug & DEBUG_INFO) == DEBUG_INFO)
		Debug::log("Done!");

	return true;
}

void to_json(const std::vector<Texture*> textures, std::vector<Sprite*> sprites, char*& json, int debug)
{
	Json::Value output;
	Json::Value textures_value;
	Json::Value sprites_value;
	for (int i = 0; i < textures.size(); i++)
	{
		Json::Value texture;
		texture["name"] = Json::Value(textures[i]->name);
		texture["width"] = Json::Value(textures[i]->width);
		texture["height"] = Json::Value(textures[i]->height);
		size_t size = wcstombs(nullptr, textures[i]->path, 0);
		assert(size != -1);
		int len = size + 1;
		char* path = new char[len];
		size_t result = wcstombs(path, textures[i]->path, len);
		assert(result != -1);
		texture["path"] = Json::Value(path);
		delete[] path;

		textures_value.append(texture);
	}
	output["textures"] = textures_value;

	for (int i = 0; i < sprites.size(); i++)
	{
		Json::Value sprite;
		sprite["name"] = Json::Value(sprites[i]->name);
		sprite["section"] = Json::Value(sprites[i]->section);
		Json::Value rect;
		rect["xMin"] = Json::Value(sprites[i]->rect.xMin);
		rect["xMax"] = Json::Value(sprites[i]->rect.xMax);
		rect["yMin"] = Json::Value(sprites[i]->rect.yMin);
		rect["yMax"] = Json::Value(sprites[i]->rect.yMax);
		sprite["rect"] = rect;
		Json::Value uv;
		uv["xMin"] = Json::Value(sprites[i]->uv.xMin);
		uv["xMax"] = Json::Value(sprites[i]->uv.xMax);
		uv["yMin"] = Json::Value(sprites[i]->uv.yMin);
		uv["yMax"] = Json::Value(sprites[i]->uv.yMax);
		sprite["uv"] = uv;

		sprites_value.append(sprite);
	}

	output["sprites"] = sprites_value;

	Json::StyledWriter writer;
	std::string str = writer.write(output);
	const char* data = str.c_str();
	assert(data != nullptr);
	copy_str(json, data);
}
