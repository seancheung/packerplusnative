#include "Debugger.h"
#include "StructDef.h"

typedef unsigned char byte;

extern "C"
{
	IMPORT bool pack(const Texture textures[], const int count, const Size max_size, const char* output_path, int bit_depth, int format,
	                 int& output_texture_count, Texture*& output_textures, int& output_sprite_count, Sprite*& output_sprites);
	IMPORT void create_empty(const int width, const int height, const char* path, int bit_depth, int format, const Color color);
	IMPORT void release(void*& pointer);
}

void convert_char(const char* input, wchar_t*& output);
void concat(const char* a, const char* b, char*& output);
void concat(const char* a, int i, char*& output);
