#include "Precompiled.h"

#if TS_PLATFORM == TS_LINUX

#include "Application.h"

#include "ts/engine/system/Commando.h"

#include "ts/file/FileList.h"

using namespace ts;

void stuff()
{
	file::FileList fl("img", file::FileListStyle_All);
	
	TS_PRINTF("\n---------------------------------------\n\n");
	
	std::vector<file::FileListEntry> all = fl.getFullListing();
	TS_UNUSED_VARIABLE(all);
	// for (const file::FileListEntry &e : all)
	// {
	// 	TS_PRINTF("%s\n", e.getFullPath());
	// }
}

int main(int numArgs, const char **argv)
{
	// stuff();
	// return 0;
	
	printf("DOING ANYTHING AT ALL!\n");
	
	printf(" Got %d args\n", numArgs);
	for (int i = 0; i < numArgs; ++i)
		printf("   %d = %s\n", i, argv[i]);
	printf("---------------------\n\n");
	
	engine::system::Commando commando;
	
	if (numArgs > 1)
	{
		if (!commando.parse(numArgs - 1, argv + 1))
			TS_LOG_WARNING("Failed to parse command line arguments.");
	}

	app::Application app(commando);
	int returnCode = app.launch();
	return returnCode;
}

#endif
