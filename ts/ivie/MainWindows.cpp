#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "Application.h"

#include "ts/tessa/common/IncludeWindows.h"
#include <shellapi.h>

#include "ts/tessa/system/Commando.h"

using namespace ts;

extern void makepack();

void mathtest()
{
	math::Mat4 mt;
	math::Quat q = math::Quat::makeFromRotation(math::VC3::up, math::VC3::forward);
	q.normalize().getMatrix(mt);

	math::VC3 euler = q.getEulerAngles();
}


// int wmain(int argc, const wchar_t **argv)
int __stdcall wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR cmdArgs, int windowShowCmd)
{
// 	makepack();
	mathtest();
	
	system::Commando commando;
	
	int32 numArgs;
	LPWSTR *argList = CommandLineToArgvW(cmdArgs, &numArgs);
	if (argList != nullptr)
	{
		// Gotta const cast here because parser expects const parameter.
		const wchar_t **argv = const_cast<const wchar_t **>(argList);

		if (numArgs > 0 && !commando.parse(numArgs, argv))
			TS_LOG_WARNING("Failed to parse command line arguments.");

		LocalFree(argList);
	}

	app::Application app(commando);
	int returnCode = app.launch();
	return returnCode;
}

#endif
