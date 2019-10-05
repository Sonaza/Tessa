#include "Precompiled.h"
#include "AbstractManagerBase.h"

TS_PACKAGE2(engine, system)

AbstractManagerBase::AbstractManagerBase()
	: gigaton(Gigaton::getSingleton())
{

}

AbstractManagerBase::~AbstractManagerBase()
{

}

bool AbstractManagerBase::isInitialized() const
{
	return initialized;
}

TS_END_PACKAGE2()
