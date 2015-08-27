#include "Debugger.h"

typedef unsigned char byte;

extern "C"
{
	IMPORT byte* test(const char* value, int* length);
}
