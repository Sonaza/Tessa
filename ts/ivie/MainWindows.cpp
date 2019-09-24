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
	/*file::FileList list;

	file::FileListStyle listStyle = file::FileListStyle_Files_Recursive;
	SizeType flags = file::FileListFlags_SkipDotEntries | file::FileListFlags_LargeFetch;
	list.open("J:\\CPP\\tessa\\workdir\\rec", listStyle, flags);

	auto files = list.getFullListing();
	for (const auto &e : files)
	{
		TS_WPRINTF("%s\n", e.getBasename());
	}*/

	system::Commando commando(cmdArgs);

// 	makepack();

	app::Application app(commando);
	int returnCode = app.launch();
	return returnCode;
}

#endif
