#include "Precompiled.h"
#include "ts/tessa/system/Timer.h"

TS_PACKAGE1(system)

sf::Clock Timer::globalTimer;

sf::Time Timer::getTime()
{
	return globalTimer.getElapsedTime();
}

TS_END_PACKAGE1()
