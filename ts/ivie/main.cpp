#include "Precompiled.h"
#include "Application.h"

#include "ts/tessa/file/FileList.h"

using namespace ts;

extern void makepack();

int wmain(int argc, const wchar_t **argv)
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

// 	makepack();
// 	return 0;

	app::Application app(argc, argv);
	int returnCode = app.launch();
	return returnCode;
}