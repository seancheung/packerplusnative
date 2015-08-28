#include "PackerPlusWrapper.h"
#include "../CXImage/ximage.h"
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
	auto wpath = convert_char(path);
	auto image = CxImage(wpath, CXIMAGE_FORMAT_UNKNOWN);
	if (wpath != nullptr)
	{
		delete[] wpath;
		wpath = nullptr;
	}
	auto pixels = image.GetBits();
	*width = image.GetWidth();
	*height = image.GetHeight();
	return pixels;
}
