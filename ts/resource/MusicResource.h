#pragma once

#include "ts/resource/ResourceBase.h"
#include "ts/resource/archivist/ArchivistInputStream.h"
#include "SFML/Audio.hpp"

TS_DECLARE1(resource, ArchivistInputStream);

TS_PACKAGE1(resource)

class MusicResource : public ResourceBase<sf::Music, TS_FOURCC('m','s','c','r')>
{
	TS_DECLARE_RESOURCE_TYPE(resource::MusicResource);

public:
	MusicResource(const String &filepath);
	~MusicResource();

protected:
	virtual bool loadResourceImpl() override;

	UniquePointer<archivist::ArchivistInputStream> strm;
};

TS_END_PACKAGE1()
