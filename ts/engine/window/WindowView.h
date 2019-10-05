#pragma once

#include "SFML/Graphics/View.hpp"

TS_PACKAGE2(engine, window)

struct WindowView
{
	math::VC2 position;    // The center position
	math::VC2 size;        // The size in pixels
	float rotation = 0.f;  // The rotation in degrees
	float scale = 1.f;     // Scales the view size in a way where >1.0 results in thigs looking smaller 

	explicit operator sf::View() const;

	math::Transform getTransform() const;

	math::VC2 convertToViewCoordinate(const math::VC2 &coordinate) const;
	math::VC2 convertFromViewCoordinate(const math::VC2 &coordinate) const;
};

TS_END_PACKAGE2()
