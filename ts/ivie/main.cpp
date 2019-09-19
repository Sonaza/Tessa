#include "Precompiled.h"
#include "Application.h"

#include <locale>
#include <codecvt>

#pragma warning( push, 3 )
#include <fmt/printf.h>
#pragma warning( pop )

#include "ts/tessa/time/Time.h"
#include "ts/tessa/time/TimeSpan.h"

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