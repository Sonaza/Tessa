#pragma once

TS_PACKAGE1(lang)

class Noncopyable
{
public:
	Noncopyable() = default;

	// Delete copy constructor and assignment
	Noncopyable(const Noncopyable &) = delete;
	Noncopyable &operator=(const Noncopyable &) = delete;
};

TS_END_PACKAGE1()
