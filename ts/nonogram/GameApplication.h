#pragma once

#include "ts/tessa/system/BaseApplication.h"

TS_DECLARE1(system, WindowManager);
TS_DECLARE1(file, ArchivistFilesystem);

TS_PACKAGE1(game)

class GameApplication : public system::BaseApplication
{
public:
	GameApplication(Int32 argc, const char **argv);
	virtual ~GameApplication();

protected:
	virtual bool sceneInitialize();
	virtual bool createWindow(system::WindowManager &windowManager);
	virtual bool loadArchives(file::ArchivistFilesystem &fileSystem);
};

TS_END_PACKAGE1()
