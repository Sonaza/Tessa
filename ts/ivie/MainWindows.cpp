#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "Application.h"
#include "ts/tessa/common/IncludeWindows.h"

#include "ts/tessa/system/Commando.h"

using namespace ts;

extern void makepack();

// int wmain(int argc, const wchar_t **argv)
int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE unused, LPWSTR cmdArgs, int windowShowCmd)
{
// 	makepack();
// 	test();
	
	system::Commando commando(cmdArgs);

	app::Application app(commando);
	int returnCode = app.launch();
	return returnCode;
}

#endif
