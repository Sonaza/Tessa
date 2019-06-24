#pragma once

#include "ts/tessa/resource/ResourceBase.h"

#include <SFML/Audio.hpp>

TS_DECLARE1(resource, ArchivistInputStream);

TS_PACKAGE1(resource)

class MusicResource : public ResourceBase<sf::Music, TS_FOURCC('m','s','c','r')>
{
	TS_DECLARE_RESOURCE_TYPE(resource::MusicResource);

public:
	MusicResource(const std::string &filepath);
	~MusicResource();

protected:
	virtual bool loadResourceImpl() override;

	UniquePointer<ArchivistInputStream> strm;
};

TS_END_PACKAGE1()