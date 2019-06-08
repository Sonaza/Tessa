#pragma once

#include "ts/tessa/system/Application.h"

TS_PACKAGE1(game)

class GameApplication : public system::Application
{
public:
	GameApplication(Int32 argc, const char **argv);
	virtual ~GameApplication();

	virtual bool sceneInitialize();

};

TS_END_PACKAGE1()
