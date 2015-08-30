#include "Debugger.h"
#include "StructDef.h"
#include <vector>

typedef unsigned char byte;

extern "C"
{
	IMPORT bool pack(const Texture textures[], const int count, const int max_width, const int max_height, const WCHAR* output_path, int bit_depth, int format,
	                 int& output_texture_count, Texture*& output_textures, int& output_sprite_count, Sprite*& output_sprites);
	IMPORT void create_empty(const int width, const int height, const WCHAR* path, int bit_depth, int format, const Color color);
	IMPORT void release(void* pointer);
}

const char* to_json(const std::vector<Texture*> textures, std::vector<Sprite*> sprites);
void concat(const WCHAR* a, const WCHAR* b, WCHAR*& output);
