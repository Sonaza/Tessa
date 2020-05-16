#pragma once

#include <SFML/Window/Mouse.hpp>

TS_PACKAGE1(input)

class Mouse
{
public:
	enum Button
	{
		Left      = sf::Mouse::Left,
		Right     = sf::Mouse::Right,
		Middle    = sf::Mouse::Middle,
		XButton1  = sf::Mouse::XButton1,
		XButton2  = sf::Mouse::XButton2,

		ButtonCount
	};
};

TS_END_PACKAGE1()

TS_PACKAGE0()

using input::Mouse;

TS_END_PACKAGE0()
