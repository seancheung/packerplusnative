#include "Debugger.h"
#include "StructDef.h"

typedef unsigned char byte;

extern "C"
{
	IMPORT void pack(const Texture* textures, const int count, const Size max_size, const char* path, Atlas* atlas, const int bit_depth, const int format);
	IMPORT void create_empty(const int width, const int height, const char* path, int bit_depth, int format, const Color color);
}

wchar_t* convert_char(const char* input);
char* append(const char* a, const char* b);
char* append(const char* a, int i);
