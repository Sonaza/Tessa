#include "Precompiled.h"
#include "RenderUtil.h"

TS_PACKAGE2(app, util)

extern sf::VertexArray makeQuadVertexArray(
	uint32 width, uint32 height,
	uint32 offsetX, uint32 offsetY)
{
	return makeQuadVertexArrayScaled(width, height, width, height, offsetX, offsetY);
}

extern sf::VertexArray makeQuadVertexArrayScaled(
	uint32 width, uint32 height,
	uint32 texWidth, uint32 texHeight,
	uint32 offsetX, uint32 offsetY)
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

extern sf::VertexArray makeQuadVertexArrayScaledShadow(
	uint32 width, uint32 height,
	uint32 texWidth, uint32 texHeight,
	float shadowOffset, const sf::Color &shadowColor,
	uint32 offsetX, uint32 offsetY)
{
	const math::VC2 size((float)width, (float)height);
	const math::VC2 texsize((float)texWidth, (float)texHeight);
	const math::VC2 offset((float)offsetX, (float)offsetY);

	sf::VertexArray va(sf::Quads, 8);

	// Shadow (darkened quad)
	va[0] = sf::Vertex(
		sf::Vector2f(offset.x + shadowOffset,          offset.y + shadowOffset),
		shadowColor,
		sf::Vector2f(0.f, 0.f)
		);
	va[1] = sf::Vertex(
		sf::Vector2f(size.x + offset.x + shadowOffset, offset.y + shadowOffset),
		shadowColor,
		sf::Vector2f(0.f, 0.f)
		);
	va[2] = sf::Vertex(
		sf::Vector2f(size.x + offset.x + shadowOffset, size.y + offset.y + shadowOffset),
		shadowColor,
		sf::Vector2f(0.f, 0.f)
		);
	va[3] = sf::Vertex(
		sf::Vector2f(offset.x + shadowOffset,          size.y + offset.y + shadowOffset),
		shadowColor,
		sf::Vector2f(0.f, 0.f)
		);

	// The quad with texture
	va[4] = sf::Vertex(
		sf::Vector2f(offset.x,          offset.y),
		sf::Vector2f(0.f,               0.f)
	);
	va[5] = sf::Vertex(
		sf::Vector2f(size.x + offset.x, offset.y),
		sf::Vector2f(texsize.x,         0.f)
	);
	va[6] = sf::Vertex(
		sf::Vector2f(size.x + offset.x, size.y + offset.y),
		sf::Vector2f(texsize.x,         texsize.y)
	);
	va[7] = sf::Vertex(
		sf::Vector2f(offset.x,          size.y + offset.y),
		sf::Vector2f(0.f,               texsize.y)
	);
	return va;
}

TS_END_PACKAGE2()