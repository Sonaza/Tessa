#pragma once

#include "ts/tessa/system/Application.h"

TS_DECLARE1(system, WindowManager);

TS_PACKAGE1(game)

class GameApplication : public system::Application
{
public:
	GameApplication(Int32 argc, const char **argv);
	virtual ~GameApplication();

protected:
	virtual bool sceneInitialize();
	virtual bool createWindow(system::WindowManager &windowManager);
};

TS_END_PACKAGE1()
