#pragma once

TS_PACKAGE1(thread)

class CurrentThread
{
public:
	CurrentThread() = delete;

	static const std::string &getThreadName();
	static SizeType getThreadId();

	static uint32 getNativeThreadId();
};

TS_END_PACKAGE1()
