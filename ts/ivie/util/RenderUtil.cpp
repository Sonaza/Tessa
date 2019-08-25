#include "Precompiled.h"
#include "RenderUtil.h"

TS_PACKAGE2(app, util)

extern sf::VertexArray makeQuadVertexArray(
	Uint32 width, Uint32 height,
	Uint32 offsetX, Uint32 offsetY)
{
	return makeQuadVertexArrayScaled(width, height, width, height, offsetX, offsetY);
}

extern sf::VertexArray makeQuadVertexArrayScaled(
	Uint32 width, Uint32 height,
	Uint32 texWidth, Uint32 texHeight,
	Uint32 offsetX, Uint32 offsetY)
{
	const math::VC2 size((float)width, (float)height);
	const math::VC2 texsize((float)texWidth, (float)texHeight);
	const math::VC2 offset((float)offsetX, (float)offsetY);

	sf::VertexArray va(sf::Quads, 4);
	va[0] = sf::Vertex(
		sf::Vector2f(offset.x,          offset.y),
		sf::Vector2f(0.f,               0.f)
	);
	va[1] = sf::Vertex(
		sf::Vector2f(size.x + offset.x, offset.y),
		sf::Vector2f(texsize.x,         0.f)
	);
	va[2] = sf::Vertex(
		sf::Vector2f(size.x + offset.x, size.y + offset.y),
		sf::Vector2f(texsize.x,         texsize.y)
	);
	va[3] = sf::Vertex(
		sf::Vector2f(offset.x,          size.y + offset.y),
		sf::Vector2f(0.f,               texsize.y)
	);
	return va;
}

TS_END_PACKAGE2()