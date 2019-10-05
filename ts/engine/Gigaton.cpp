#include "Precompiled.h"
#include "ts/engine/Gigaton.h"

TS_PACKAGE1(engine)

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
