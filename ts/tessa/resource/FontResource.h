#pragma once

#include "ts/tessa/resource/ResourceBase.h"

TS_PACKAGE1(resource)

class FontResource : public ResourceBase<sf::Font>
{
public:
	FontResource(const std::string &filepath);
	~FontResource();

	bool loadResource();
};

TS_END_PACKAGE1()
