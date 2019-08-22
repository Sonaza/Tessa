#include "Precompiled.h"
#include "RenderUtil.h"

TS_PACKAGE2(app, util)

extern sf::VertexArray makeQuadVertexArray(Uint32 width, Uint32 height, Uint32 offsetX, Uint32 offsetY)
{
	math::VC2 size((float)width, (float)height);
	math::VC2 offset((float)offsetX, (float)offsetY);

	sf::VertexArray va(sf::Quads, 4);
	va[0] = sf::Vertex(
		sf::Vector2f(offset.x,          offset.y),
		sf::Vector2f(0.f,               0.f)
	);
	va[1] = sf::Vertex(
		sf::Vector2f(size.x + offset.x, offset.y),
		sf::Vector2f(size.x,            0.f)
	);
	va[2] = sf::Vertex(
		sf::Vector2f(size.x + offset.x, size.y + offset.y),
		sf::Vector2f(size.x,            size.y)
	);
	va[3] = sf::Vertex(
		sf::Vector2f(offset.x,          size.y + offset.y),
		sf::Vector2f(0.f,               size.y)
	);
	return va;
}

TS_END_PACKAGE2()