#include "Precompiled.h"
#include "GameApplication.h"

#include "ts/tessa/common/Log.h"

int main(int argc, const char **argv)
{
	TS_ASSERTF(false, "This is my custom text in the assertion message %d", 1337);

	ts::game::GameApplication app(argc, argv);
	int returnCode = app.start();
	return returnCode;
}