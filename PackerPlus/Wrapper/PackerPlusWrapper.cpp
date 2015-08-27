#include "PackerPlusWrapper.h"


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
