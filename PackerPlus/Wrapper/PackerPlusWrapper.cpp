#include "PackerPlusWrapper.h"
#include "../CXImage/ximage.h"
#include <string>


byte* test(const char* value, int* length)
{
	Debug::log(value);
	*length = 10;
	byte* bytes = new byte[*length];
	for (int i = 0; i < *length; i++)
	{
		bytes[i] = i;
	}
	return bytes;
}

wchar_t* convert_char(const char* input)
{
	int len = MultiByteToWideChar(CP_ACP, 0, input, -1, nullptr, 0);
	wchar_t* output = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, input, -1, output, len);
	return output;
}

byte* ReadImage(const char* path, long* width, long* height)
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
