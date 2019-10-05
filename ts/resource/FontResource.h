#pragma once

#include "ts/resource/ResourceBase.h"
#include "ts/resource/archivist/ArchivistInputStream.h"
#include "SFML/Graphics/Font.hpp"

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

	UniquePointer<archivist::ArchivistInputStream> strm;
};

TS_END_PACKAGE1()
