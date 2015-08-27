#include "DLLAPI.h"

typedef void (_stdcall*debugcall)(const char*);
static debugcall _log, _error, _warning;

class Debug
{
public:
	static void log(const char* content)
	{
		if (_log != nullptr)
			_log(content);
	}

	static void error(const char* content)
	{
		if (error != nullptr)
			_error(content);
		else
			log(content);
	}

	static void warning(const char* content)
	{
		if (_warning != nullptr)
			_warning(content);
		else
			log(content);
	}
};

extern "C" inline EXPORT void link_debug(debugcall log, debugcall error, debugcall warning)
{
	_log = log;
	_error = error;
	_warning = warning;

	Debug::log("Debug linked");
}
