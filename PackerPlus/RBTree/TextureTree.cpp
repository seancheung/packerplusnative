#include "TextureTree.h"

TextureTree::TextureTree()
{
	name = nullptr;
	image = nullptr;
	bleed = false;
	_filled = false;
	padding = 2;
}

TextureTree::TextureTree(const Rect<int> rect)
{
	this->rect = Rect<int>(rect);
	name = nullptr;
	image = nullptr;
	bleed = false;
	_filled = false;
	padding = 2;
}

TextureTree::~TextureTree()
{
	if (name != nullptr)
		delete[] name;
	if (image != nullptr)
		delete[] image;
}

bool TextureTree::add_texture(CxImage* input, int index, const char* name)
{
	if (input == nullptr)
		return false;
	if (has_children())
	{
		/*find the deepest node*/
		if (left->add_texture(input, index, name))
			return true;
		return right->add_texture(input, index, name);
	}

	/*already filled*/
	if (_filled)
		return false;
	int rw = rect.width();
	int rh = rect.height();
	int tw = input->GetWidth();
	int th = input->GetHeight();

	/*no available space to fit*/
	if (rw < tw || rh < th)
		return false;

	/*Perfectly fit*/
	if (rw == tw && rh == th)
	{
		image = input;
		_filled = true;
		this->index = index;
		this->name = new char[128];
		strcpy(this->name, name);
		return this;
	}

	/*split*/
	init_children();

	/*calculate alignment*/
	int deltaw = rw - tw;
	int deltah = rh - th;
	/*horizontal*/
	if (deltaw > deltah)
	{
		left->rect = Rect<int>(rect.xMin, rect.yMin, tw, rh);
		right->rect = Rect<int>(rect.xMin + tw + padding, rect.yMin, rw - tw - padding, rh);
	}
	/*vertical*/
	else
	{
		left->rect = Rect<int>(rect.xMin, rect.yMin, rw, th);
		right->rect = Rect<int>(rect.xMin, rect.yMin + th + padding, rw, rh - th - padding);
	}

	return left->add_texture(input, index, name);
}

void TextureTree::get_bounds(std::vector<TextureTree*>& bounds)
{
	std::vector<TextureTree*> children;
	get_children(children);
	for (TextureTree* child : children)
	{
		if (child != nullptr && child->image != nullptr)
			bounds.push_back(child);
	}
}

void TextureTree::get_names(std::vector<char*>& names)
{
	std::vector<TextureTree*> bounds;
	get_bounds(bounds);
	for (TextureTree* bound : bounds)
	{
		names.push_back(bound->name);
	}
}

void TextureTree::build(CxImage& output)
{
	/*build children*/
	if (has_children())
	{
		left->build(output);
		right->build(output);
	}
	if (image != nullptr)
	{
		int tw = image->GetWidth();
		int th = image->GetHeight();
		bool alpha = image->AlphaIsValid();

		for (size_t x = 0; x < tw; x++)
		{
			for (size_t y = 0; y < th; y++)
			{
				RGBQUAD pixel = image->GetPixelColor(x, y, alpha);
				output.SetPixelColor(x + rect.xMin, y + rect.yMin, pixel, alpha);
			}
		}

		/*apply bleeding*/
		if (bleed && padding)
		{
			for (size_t y = 0; y < th; y++)
			{
				int x = tw - 1;
				RGBQUAD pixel = image->GetPixelColor(x, y, alpha);
				output.SetPixelColor(x + rect.xMin + padding, y + rect.yMin, pixel, alpha);
			}
			for (size_t x = 0; x < th; x++)
			{
				int y = th - 1;
				RGBQUAD pixel = image->GetPixelColor(x, y, alpha);
				output.SetPixelColor(x + rect.xMin, y + rect.yMin + padding, pixel, alpha);
			}
		}
	}
}

bool TextureTree::operator<(const TextureTree& tree) const
{
	return index < tree.index;
}
