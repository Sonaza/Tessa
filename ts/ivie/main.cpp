#include "Precompiled.h"
#include "Application.h"

// #include "ts/tessa/file/FileUtils.h"

using namespace ts;

extern void makepack();

int wmain(int argc, const wchar_t **argv)
{
// 	makepack();
// 	return 0;

	app::Application app(argc, argv);
	int returnCode = app.launch();
	return returnCode;
}