#include "Precompiled.h"
#include "ts/tessa/system/AbstractManagerBase.h"

TS_PACKAGE1(system)

AbstractManagerBase::AbstractManagerBase()
	: gigaton(system::Gigaton::getSingleton())
{

}

AbstractManagerBase::~AbstractManagerBase()
{

}

bool AbstractManagerBase::isInitialized() const
{
	return initialized;
}

TS_END_PACKAGE1()
