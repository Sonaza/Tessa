#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
extern "C" __declspec(dllexport) unsigned long AmdPowerXpressRequestHighPerformance = 1;

#endif
