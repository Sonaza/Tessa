#include "Precompiled.h"

#if TS_PLATFORM == TS_LINUX

#include "Application.h"

#include "ts/engine/system/Commando.h"

using namespace ts;

// int main(int numArgs, const char **argv)
// {
// 	printf("Hello World!\n");
// 	return 0;
// }

int main(int numArgs, const char **argv)
{
	printf("DOING ANYTHING AT ALL!\n");
	
	engine::system::Commando commando;
	
	if (!commando.parse(numArgs, argv))
		TS_LOG_WARNING("Failed to parse command line arguments.");

	app::Application app(commando);
	int returnCode = app.launch();
	return returnCode;
}

#endif
