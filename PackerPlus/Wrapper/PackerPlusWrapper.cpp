#include "PackerPlusWrapper.h"
#include "../RBTree/TextureTree.h"
#include <string>
#include <algorithm>
#include "../jsoncpp/json/writer.h"

void concat(const WCHAR* a, int i, WCHAR*& output)
{
	const WCHAR* dot = _tcschr(a, '.');
	if (!dot || dot == a)
	_tcscpy(output, (std::wstring(a) + std::to_wstring(i)).c_str());
	else
	{
		std::wstring str = std::wstring(a);
		int index = str.find_last_of('.');
		_tcscpy(output, (str.substr(0, index) + _T("_") + std::to_wstring(i) + std::wstring(dot + 1)).c_str());
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

bool pack(const Texture textures[], const int count, const int max_width, const int max_height, const WCHAR* output_path, int bit_depth, int format, int& output_texture_count, Texture*& output_textures, int& output_sprite_count, Sprite*& output_sprites)
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

	output_texture_count = trees.size();
	/*TODO: call extern delete for managed data*/
	output_textures = new Texture[output_texture_count];

	std::vector<Sprite*> sprites;
	for (int i = 0; i < trees.size(); i++)
	{
		output_textures[i].name = new char[128];
		strcpy(output_textures[i].name, std::to_string(i).c_str());
		output_textures[i].path = new WCHAR[1024];
		if (i > 0)
			concat(output_path, i, output_textures[i].path);
		else
		_tcscpy(output_textures[i].path, output_path);
		/*color depth and image format*/
		CxImage image = CxImage(max_width, max_height, bit_depth, format);
		if (image.AlphaCreate())
			image.AlphaSet(0);
		trees[i]->build(image);
		/*TODO: apply crop?*/
		int rw = trees[i]->get_root_width();
		int rh = trees[i]->get_root_height();
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
			sprite->name = new char[128];
			strcpy(sprite->name, bound->name);
			sprite->uv = uv;
			sprite->section = i;
			sprites.push_back(sprite);
		}

		bool result = image.Save(output_textures[i].path, format);

		if (result)
		{
			Debug::log("Successfully saved");
		}
		else
		{
			Debug::error("Failed to save");
		}
	}

	std::vector<TextureTree*>::iterator tree;
	for (tree = trees.begin(); tree != trees.end(); ++tree)
	{
		delete *tree;
	}
	trees.clear();

	output_sprite_count = sprites.size();
	/*TODO: delete call*/
	output_sprites = new Sprite[output_sprite_count];
	for (int i = 0; i < output_sprite_count; i++)
	{
		output_sprites[i].name = new char[128];
		strcpy(output_sprites[i].name, sprites[i]->name);
		output_sprites[i].section = sprites[i]->section;
		output_sprites[i].rect = sprites[i]->rect;
		output_sprites[i].uv = sprites[i]->uv;
	}

	std::vector<Sprite*>::iterator sprite;
	for (sprite = sprites.begin(); sprite != sprites.end(); ++sprite)
	{
		delete *sprite;
	}
	sprites.clear();

	return true;
}

const char* to_json(const std::vector<Texture*> textures, std::vector<Sprite*> sprites)
{
	Json::Value output;
	Json::Value tv, sv;
	output.append(tv);
	output.append(sv);
	for (Texture* texture : textures)
	{
		Json::Value t;
		tv.append(t);
		t["name"] = texture->name;
		t["path"] = texture->path;
	}
	for (Sprite* sprite : sprites)
	{
		Json::Value s;
		sv.append(s);
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
	}

	return output.toStyledString().c_str();
}

void release(void* pointer)
{
	if (pointer != nullptr)
		delete pointer;	
}
