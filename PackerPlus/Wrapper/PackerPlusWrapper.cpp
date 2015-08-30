#include "PackerPlusWrapper.h"
#include "../RBTree/TextureTree.h"
#include <string>
#include <algorithm>
#include "../jsoncpp/json/writer.h"

void concat(const WCHAR* a, int i, WCHAR*& output)
{
	const WCHAR* dot = _tcschr(a, '.');
	if (!dot || dot == a)
	{
		const WCHAR* copy = (std::wstring(a) + std::to_wstring(i)).c_str();
		int len = wcslen(copy) + sizeof(WCHAR);
		_tcscpy_s(output, len, copy);
	}
	else
	{
		std::wstring str = std::wstring(a);
		int index = str.find_last_of('.');
		const WCHAR* copy = (str.substr(0, index) + _T("_") + std::to_wstring(i) + std::wstring(dot + 1)).c_str();
		int len = wcslen(copy) + sizeof(WCHAR);
		_tcscpy_s(output, len, copy);
	}
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
		auto name = std::to_string(i).c_str();
		int nlen = strlen(name) + sizeof(char);
		texture->name = new char[nlen];
		strcpy_s(texture->name, nlen, name);
		if (i > 0)
			concat(output_path, i, texture->path);
		else
		{
			int plen = wcslen(output_path) + sizeof(WCHAR);
			texture->path = new WCHAR[plen];
			_tcscpy_s(texture->path, plen, output_path);
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
		int w = image.GetWidth();
		int h = image.GetHeight();

		std::vector<TextureTree*> bounds;
		trees[i]->get_bounds(bounds);
		sort(bounds.begin(), bounds.end());
		for (TextureTree* bound : bounds)
		{
			UVRect uv = UVRect(
				float(bound->rect.xMin) / float(w),
				float(bound->rect.yMin) / float(h),
				float(bound->rect.width()) / float(w),
				float(bound->rect.height()) / float(h));
			Sprite* sprite = new Sprite();
			sprite->rect = bound->rect;
			int len = strlen(bound->name) + sizeof(char);
			sprite->name = new char[len];
			strcpy_s(sprite->name, len, bound->name);
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
	Json::Value tv, sv;
	for (Texture* texture : textures)
	{
		Json::Value t;		
		t["name"] = texture->name;
		t["path"] = texture->path;
		tv.append(t);
	}
	for (Sprite* sprite : sprites)
	{
		Json::Value s;
		s["name"] = sprite->name;
		s["section"] = sprite->section;
		s["rect"]["xMin"] = sprite->rect.xMin;
		s["rect"]["xMax"] = sprite->rect.xMax;
		s["rect"]["yMin"] = sprite->rect.yMin;
		s["rect"]["yMax"] = sprite->rect.yMax;
		s["uv"]["xMin"] = sprite->uv.xMin;
		s["uv"]["xMax"] = sprite->uv.xMax;
		s["uv"]["yMin"] = sprite->uv.yMin;
		s["uv"]["yMax"] = sprite->uv.yMax;
		sv.append(s);
	}

	output["textures"] = tv;
	output["sprites"] = sv;
	const char* data = output.toStyledString().c_str();
	int len = strlen(data) + sizeof(char);
	json = new char[len];
	strcpy_s(json, len, data);
}

void release(void* pointer)
{
	if (pointer != nullptr)
		delete pointer;
}
