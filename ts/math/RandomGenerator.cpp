#include "Precompiled.h"
#include "ts/math/RandomGenerator.h"

#include "ts/thread/Mutex.h"
#include "ts/thread/MutexGuard.h"
#include <random>

TS_PACKAGE1(math)

struct RandomDevice
{
	Mutex mutex;
	std::random_device randomDevice;
};

static RandomDevice &getRandomDevice()
{
	static RandomDevice randomDevice;
	return randomDevice;
}

uint32_t generateRandom32()
{
	RandomDevice &device = getRandomDevice();
	MutexGuard mg(device.mutex);
	return device.randomDevice();
}

uint64_t generateRandom64()
{
	RandomDevice &device = getRandomDevice();
	MutexGuard mg(device.mutex);
	uint64_t result = device.randomDevice();
	result <<= 32;
	result |= device.randomDevice();
	return result;
}

TS_END_PACKAGE1()
