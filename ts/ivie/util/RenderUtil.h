#pragma once

TS_PACKAGE2(app, util)

extern sf::VertexArray makeQuadVertexArray(
	uint32_t width, uint32_t height,
	uint32_t offsetX = 0, uint32_t offsetY = 0);

extern sf::VertexArray makeQuadVertexArrayScaled(
	uint32_t width, uint32_t height,
	uint32_t texWidth, uint32_t texHeight,
	uint32_t offsetX = 0, uint32_t offsetY = 0);

extern sf::VertexArray makeQuadVertexArrayScaledShadow(
	uint32_t width, uint32_t height,
	uint32_t texWidth, uint32_t texHeight,
	float shadowOffset, const sf::Color &shadowColor,
	uint32_t offsetX = 0, uint32_t offsetY = 0);

TS_END_PACKAGE2()
