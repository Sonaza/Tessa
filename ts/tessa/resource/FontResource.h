#pragma once

#include "ts/tessa/resource/ResourceBase.h"

TS_PACKAGE1(resource)

class FontResource : public ResourceBase<sf::Font>
{
public:
	static const SizeType TypeId = FOUR_CC('f','n','t','r');

	FontResource(const std::string &filepath);
	~FontResource();

protected:
	virtual bool loadResourceImpl() override;
};

TS_END_PACKAGE1()
