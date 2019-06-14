#include "Precompiled.h"
#include "ts/tessa/system/Gigaton.h"

TS_PACKAGE1(system)

Gigaton &Gigaton::getSingleton()
{
	static Gigaton instance;
	return instance;
}

Gigaton::Gigaton()
{
}

Gigaton::~Gigaton()
{
}

void Gigaton::unregisterAll()
{
	instances.clear();
}

TS_END_PACKAGE1()
