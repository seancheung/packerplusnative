#include "Debugger.h"

typedef unsigned char byte;

extern "C"
{
	IMPORT byte* test(const char* value, int* length);
	IMPORT byte* ReadImage(const char* path, long* width, long* height);
}

wchar_t* convert_char(const char *input);
