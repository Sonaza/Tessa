#include "Precompiled.h"

#include <cassert>

#if TS_PLATFORM == TS_WINDOWS
	#include "ts/tessa/common/IncludeWindows.h"
	#include <codecvt>
	#include <sstream>
#endif

TS_PACKAGE0()

namespace
{

enum DialogAction
{
	Debug,
	Ignore,
	Abort,
};

DialogAction dialog(const char *message, const char *filepath, const unsigned int line)
{
#if TS_PLATFORM == TS_WINDOWS
	std::wstringstream msg;
	msg << "Assertion failed in " << filepath << " on line " << line << "\n\n" << message;

	Int32 button = MessageBox(nullptr, msg.str().c_str(), L"Assertion failure", MB_ABORTRETRYIGNORE | MB_ICONERROR);
	switch (button)
	{
		case IDABORT: return Abort;
		case IDRETRY: return Debug;
		case IDIGNORE: return Ignore;
	}
#endif
	return Debug;
}

}

void __cdecl breakpoint()
{
	#if TS_PLATFORM == TS_WINDOWS
		DebugBreak();
	#elif TS_PLATFORM == TS_LINUX
		__builtin_trap();
	#elif
		#error "Unsupported platform."
	#endif
}

void __cdecl assert_impl(const char *message, const char *filepath, const unsigned int line)
{
	TS_PRINTF("Assertion failed: %s in %s on line %u\n", message, filepath, line);
	
	DialogAction action = dialog(message, filepath, line);
	switch (action)
	{
		case Debug:
			breakpoint();
			return;

		case Ignore:
			// ASDASD
			return;

		case Abort:
			exit(1337);
			return;
	}
}

TS_END_PACKAGE0()
