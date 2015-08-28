#include "TextureTree.h"

TextureTree::TextureTree()
{
}

TextureTree::TextureTree(const Rect<int> rect)
{
	this->rect = new Rect<int>(rect);
}

TextureTree* TextureTree::add_texture(CxImage* texture, int index, const char* name)
{
	if (texture == nullptr)
		return nullptr;
	if (has_children())
	{
		/*find the deepest node*/
		TextureTree* complete = left->add_texture(texture, index, name);
		if (complete != nullptr)
			return complete;
		return right->add_texture(texture, index, name);
	}

	/*already filled*/
	if (_filled)
		return nullptr;
	int rw = rect->width();
	int rh = rect->height();
	int tw = texture->GetWidth();
	int th = texture->GetHeight();

	/*no available space to fit*/
	if (rw < tw || rh < th)
		return nullptr;

	/*Perfectly fit*/
	if (rw == tw && rh == th)
	{
		this->texture = texture;
		_filled = 1;
		this->index = index;
		this->name = const_cast<char*>(name);
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
		left->rect = new Rect<int>(rect->xMin, rect->yMin, tw, rh);
		right->rect = new Rect<int>(rect->xMin + tw + padding, rect->yMin, rw - tw - padding, rh);
	}
	/*vertical*/
	else
	{
		left->rect = new Rect<int>(rect->xMin, rect->yMin, rw, th);
		right->rect = new Rect<int>(rect->xMin, rect->yMin + th + padding, rw, rh - th - padding);
	}

	return left->add_texture(texture, index, name);
}

std::vector<TextureTree*> TextureTree::get_bounds()
{
	std::vector<TextureTree*> v;
	for (TextureTree* child : get_children())
	{
		if (child != nullptr && child->_filled)
			v.push_back(child);
	}
	return v;
}

std::vector<char*> TextureTree::get_names()
{
	std::vector<char*> v;
	for (TextureTree* bound : get_bounds())
	{
		if (bound != nullptr && bound->_filled)
			v.push_back(bound->name);
	}
	return v;
}

void TextureTree::build(CxImage* output)
{
	/*build children*/
	if (has_children())
	{
		left->build(output);
		right->build(output);
	}
	if (texture != nullptr)
	{
		int tw = texture->GetWidth();
		int th = texture->GetHeight();

		for (size_t x = 0; x < tw; x++)
		{
			for (size_t y = 0; y < th; y++)
			{
				RGBQUAD pixel = texture->GetPixelColor(x, y, true);
				output->SetPixelColor(x + rect->xMin, y + rect->yMin, pixel, true);
			}
		}

		/*apply bleeding*/
		if (bleed && padding)
		{
			for (size_t y = 0; y < th; y++)
			{
				int x = tw - 1;
				RGBQUAD pixel = texture->GetPixelColor(x, y, true);
				output->SetPixelColor(x + rect->xMin + padding, y + rect->yMin, pixel, true);
			}
			for (size_t x = 0; x < th; x++)
			{
				int y = th - 1;
				RGBQUAD pixel = texture->GetPixelColor(x, y, true);
				output->SetPixelColor(x + rect->xMin, y + rect->yMin + padding, pixel, true);
			}
		}
	}
}

void TextureTree::dispose_children()
{
	if (texture != nullptr)
	{
		delete[] texture;
		texture = nullptr;
	}
	RBTree<TextureTree>::dispose_children();
}