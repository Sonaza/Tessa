#include "Precompiled.h"
#include "ts/nonogram/GameApplication.h"

#include "ts/nonogram/scenes/NonogramScene.h"

TS_PACKAGE1(game)

GameApplication::GameApplication()
{
}

GameApplication::~GameApplication()
{
}

bool GameApplication::sceneInitialize()
{
	return loadScene<scenes::NonogramScene>();
}

TS_END_PACKAGE1()
