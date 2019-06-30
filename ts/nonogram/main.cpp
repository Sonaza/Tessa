#include "Precompiled.h"
#include "GameApplication.h"

extern void randomtests();

// #if TS_PLATFORM == TS_WINDOWS
// #include "ts/tessa/common/IncludeWindows.h"

// int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
// #else
int main(int argc, const char **argv)
// #endif
{
// 	randomtests();
// 	return r;

	ts::game::GameApplication app(argc, argv);
	int returnCode = app.start();
	return returnCode;
}