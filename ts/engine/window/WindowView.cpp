#include "Precompiled.h"
#include "WindowView.h"

TS_PACKAGE2(engine, window)

WindowView::operator sf::View() const
{
	sf::View view;
	view.setSize(size * scale);
	view.setCenter(position.x, position.y);
	view.setRotation(rotation);
	return view;
}

math::Transform WindowView::getTransform() const
{
	math::Transform t;
	t.translate(-size / 2.f + position / 2.f)
		.scale(scale, scale)
		.rotate(rotation);
	return t;
}

math::VC2 WindowView::convertToViewCoordinate(const math::VC2 &coordinate) const
{
	return getTransform().transformPoint(coordinate);
}

math::VC2 WindowView::convertFromViewCoordinate(const math::VC2 &coordinate) const
{
	return getTransform().getInverse().transformPoint(coordinate);
}

TS_END_PACKAGE2()
