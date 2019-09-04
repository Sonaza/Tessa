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

int asdfasd()
{
	Time t = Time::now();
	TS_PRINTF("%s", t.fromEpoch().getAsString());

	return 1;
}

int wmain(int argc, const wchar_t **argv)
{
// 	if (asdfasd())
// 		return 0;

	app::Application app(argc, argv);
	int returnCode = app.launch();
	return returnCode;
}