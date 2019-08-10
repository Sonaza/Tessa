#pragma once

#include <thread>

TS_PACKAGE2(threading, utils)

extern void setThreadName(std::thread &thread, const std::string &threadName);
extern void setCurrentThreadName(const std::string &threadName);

enum ThreadPriority
{
	ThreadPriority_Critical,
	ThreadPriority_High,
	ThreadPriority_AboveNormal,
	ThreadPriority_Normal,
	ThreadPriority_BelowNormal,
	ThreadPriority_Low,
	ThreadPriority_Idle,

};
extern void setThreadPriority(std::thread &thread, ThreadPriority priority);
extern void setCurrentThreadPriority(ThreadPriority priority);

TS_END_PACKAGE2()
