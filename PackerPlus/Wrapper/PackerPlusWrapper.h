#include "Debugger.h"
#include "StructDef.h"
#include <vector>

typedef unsigned char byte;

extern "C"
{
	/*pack txtures into one or more atlas textures, return json string*/
	IMPORT bool pack(const Texture textures[], const int count, Options option, char*& output, const int debug);
	/*create an empty texture*/
	IMPORT void create_empty(const int width, const int height, const WCHAR* path, int bit_depth, int format, const Color color);
}

/*get json string*/
void to_json(const std::vector<Texture*> textures, std::vector<Sprite*> sprites, char*& json, int debug);
/*insert index before file extension*/
void concat_path(WCHAR*& dest, const WCHAR* src, int suffix);
/*copy src to dest*/
void copy_str(char*& dest, const char* src);
/*copy src to dest*/
void copy_str(WCHAR*& dest, const WCHAR* src);
/*concat two string*/
void concat_str(WCHAR*& dest, const WCHAR* src);

template <typename T>
void free_vector(std::vector<T*>& vector)
{
	typename std::vector<T*>::iterator t;
	for (t = vector.begin(); t != vector.end(); ++t)
	{
		if (*t != nullptr)
			delete *t;
	}
	vector.clear();
}
