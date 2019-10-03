#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "Application.h"
#include "ts/tessa/common/IncludeWindows.h"

#include "ts/tessa/system/Commando.h"

using namespace ts;

extern void makepack();

void mathtest()
{
	math::Mat4 mt;
	math::Quat q = math::Quat::makeFromRotation(math::VC3::up, math::VC3::forward);
	q.normalize().getMatrix(mt);
}


// int wmain(int argc, const wchar_t **argv)
int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE unused, LPWSTR cmdArgs, int windowShowCmd)
{
// 	makepack();
	mathtest();
	
	system::Commando commando(cmdArgs);

	app::Application app(commando);
	int returnCode = app.launch();
	return returnCode;
}

#endif
