#include "Precompiled.h"
#include "ts/tessa/system/SceneBase.h"

TS_PACKAGE1(system)

SceneBase::SceneBase(system::Application *application)
	: application(application)
{
}

SceneBase::~SceneBase()
{
}

bool SceneBase::internalStart()
{
	if (start())
	{
		sceneLoaded = true;
		return true;
	}
	return false;
}

void SceneBase::internalStop()
{
	stop();
	sceneLoaded = false;

}

TS_END_PACKAGE1()
