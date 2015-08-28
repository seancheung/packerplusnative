#include "PackerPlusWrapper.h"
#include "../CXImage/ximage.h"
#include "../RBTree/TextureTree.h"
#include <string>


wchar_t* convert_char(const char* input)
{
	int len = MultiByteToWideChar(CP_ACP, 0, input, -1, nullptr, 0);
	wchar_t* output = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, input, -1, output, len);
	return output;
}

byte* read_image(const char* path, long* width, long* height)
{
	wchar_t* wpath = convert_char(path);
	CxImage image = CxImage(wpath, CXIMAGE_FORMAT_UNKNOWN);
	if (wpath != nullptr)
	{
		delete[] wpath;
		wpath = nullptr;
	}
	BYTE* pixels = image.GetBits();
	*width = image.GetWidth();
	*height = image.GetHeight();
	return pixels;
}

bool pack(const char** paths, const int count, const int width, const int height)
{
	if (paths == nullptr)
		return false;

	std::vector<CxImage> textures;
	/*load images*/
	while (paths != nullptr)
	{
		wchar_t* path = convert_char(*paths);
		CxImage image = CxImage(path, CXIMAGE_FORMAT_PNG);
		textures.push_back(image);
		paths++;
	}
	reverse(textures.begin(), textures.end());

	int index = count - 1;
	Rect<int>* rect = new Rect<int>(0, 0, width, height);
	std::vector<TextureTree*> trees;
	while (index >= 0)
	{
		TextureTree* tree = new TextureTree(*rect);
		trees.push_back(tree);
		int tindex = 0;
		while (index >= 0)
		{
			for (TextureTree* node : trees)
			{
				if (node->add_texture(textures[index], tindex++, const_cast<char*>(paths[count - index - 1])))
				{
					Debug::log("Multiple Textures generated");
					break;
				}
				index--;
			}
		}
	}

	/*sprite infos*/
	/*...*/

	for (TextureTree* node : trees)
	{

	}
}
