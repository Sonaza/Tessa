#include "Precompiled.h"
#include "ts/tessa/system/AbstractSceneBase.h"

TS_PACKAGE1(system)

AbstractSceneBase::AbstractSceneBase(system::BaseApplication *application)
	: application(application)
{
}

AbstractSceneBase::~AbstractSceneBase()
{
}

bool AbstractSceneBase::internalStart()
{
	if (start())
	{
		sceneLoaded = true;
		return true;
	}
	return false;
}

void AbstractSceneBase::internalStop()
{
	stop();
	sceneLoaded = false;
}

TS_END_PACKAGE1()
