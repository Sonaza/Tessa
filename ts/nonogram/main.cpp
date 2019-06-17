#include "Precompiled.h"
#include "GameApplication.h"

extern void randomtests();

int main(int argc, const char **argv)
{
// 	randomtests();
// 	return 0;

	ts::game::GameApplication app(argc, argv);
	int returnCode = app.start();
	return returnCode;
}