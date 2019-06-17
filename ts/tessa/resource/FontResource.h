#pragma once

#include "ts/tessa/resource/ResourceBase.h"

TS_PACKAGE1(resource)

class FontResource : public ResourceBase<sf::Font, TS_FOURCC('f','n','t','r')>
{
	TS_DECLARE_RESOURCE_TYPE(resource::FontResource);

public:
	FontResource(const std::string &filepath);
	~FontResource();

protected:
	virtual bool loadResourceImpl() override;
};

TS_END_PACKAGE1()
