#include "Debugger.h"
#include "StructDef.h"
#include <vector>

typedef unsigned char byte;

extern "C"
{
	/*pack txtures into one or more atlas textures, return json string*/
	IMPORT bool pack(const Texture textures[], const int count, const int max_width, const int max_height, const WCHAR* output_path, int bit_depth, int format, char*& output);
	/*create an empty texture*/
	IMPORT void create_empty(const int width, const int height, const WCHAR* path, int bit_depth, int format, const Color color);
}

/*get json string*/
void to_json(const std::vector<Texture*> textures, std::vector<Sprite*> sprites, char*& json);
/*insert index before file extension*/
void concat_path(const WCHAR* a, int i, WCHAR*& output);
/*copy src to dest*/
void copy_str(char*& dest, const char* src);
/*copy src to dest*/
void copy_str(WCHAR*& dest, const WCHAR* src);
