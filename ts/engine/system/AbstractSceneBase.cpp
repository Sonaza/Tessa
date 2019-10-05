#include "Precompiled.h"
#include "ts/engine/system/AbstractSceneBase.h"

TS_PACKAGE2(engine, system)

AbstractSceneBase::AbstractSceneBase(engine::system::BaseApplication *application)
	: application(application)
	, gigaton(Gigaton::getSingleton())
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

TS_END_PACKAGE2()
