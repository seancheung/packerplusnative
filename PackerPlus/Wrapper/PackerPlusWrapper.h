#include "Debugger.h"

typedef unsigned char byte;

extern "C"
{
	IMPORT byte* read_image(const char* path, long* width, long* height);
}

wchar_t* convert_char(const char *input);
