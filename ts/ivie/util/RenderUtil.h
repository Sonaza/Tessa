#pragma once

TS_PACKAGE2(app, util)

extern sf::VertexArray makeQuadVertexArray(
	uint32 width, uint32 height,
	uint32 offsetX = 0, uint32 offsetY = 0);

extern sf::VertexArray makeQuadVertexArrayScaled(
	uint32 width, uint32 height,
	uint32 texWidth, uint32 texHeight,
	uint32 offsetX = 0, uint32 offsetY = 0);

extern sf::VertexArray makeQuadVertexArrayScaledShadow(
	uint32 width, uint32 height,
	uint32 texWidth, uint32 texHeight,
	float shadowOffset, const sf::Color &shadowColor,
	uint32 offsetX = 0, uint32 offsetY = 0);

TS_END_PACKAGE2()
