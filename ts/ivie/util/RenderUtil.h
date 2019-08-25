#pragma once

TS_PACKAGE2(app, util)

extern sf::VertexArray makeQuadVertexArray(
	Uint32 width, Uint32 height,
	Uint32 offsetX = 0, Uint32 offsetY = 0);

extern sf::VertexArray makeQuadVertexArrayScaled(
	Uint32 width, Uint32 height,
	Uint32 texWidth, Uint32 texHeight,
	Uint32 offsetX = 0, Uint32 offsetY = 0);

extern sf::VertexArray makeQuadVertexArrayScaledShadow(
	Uint32 width, Uint32 height,
	Uint32 texWidth, Uint32 texHeight,
	float shadowOffset, const sf::Color &shadowColor,
	Uint32 offsetX = 0, Uint32 offsetY = 0);

TS_END_PACKAGE2()
