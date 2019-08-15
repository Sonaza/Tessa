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

TS_END_PACKAGE1()
