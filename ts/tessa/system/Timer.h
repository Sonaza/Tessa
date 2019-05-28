#pragma once

TS_PACKAGE1(system)

class Timer
{
public:
	static sf::Time getTime();

private:
	Timer();
	~Timer();

	static sf::Clock globalTimer;
};

TS_END_PACKAGE1()
