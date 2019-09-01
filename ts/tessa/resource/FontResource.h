#pragma once

#include "ts/tessa/resource/ResourceBase.h"

TS_DECLARE1(resource, ArchivistInputStream);

TS_PACKAGE1(resource)

class FontResource : public ResourceBase<sf::Font, TS_FOURCC('f','n','t','r')>
{
	TS_DECLARE_RESOURCE_TYPE(resource::FontResource);

public:
	FontResource(const String &filepath);
	~FontResource();

protected:
	virtual bool loadResourceImpl() override;

	UniquePointer<ArchivistInputStream> strm;
};

TS_END_PACKAGE1()
