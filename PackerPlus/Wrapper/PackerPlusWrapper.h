#include "Debugger.h"
#include "StructDef.h"

typedef unsigned char byte;

extern "C"
{
	IMPORT void pack(const PackData* textures, const int count, const Size max_size, const char* path, Atlas* atlas);
	IMPORT void create_empty(const int width, const int height, const char* path);
}

wchar_t* convert_char(const char* input);
char* append(const char* a, const char* b);
char* append(const char* a, int i);
