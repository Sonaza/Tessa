#pragma once

#include "ts/resource/ResourceBase.h"

#include "SFML/Audio.hpp"

TS_PACKAGE1(resource)

class SoundResource : public resource::ResourceBase<sf::SoundBuffer, TS_FOURCC('s','n','d','r')>
{
	TS_DECLARE_RESOURCE_TYPE(resource::SoundResource);

public:
	SoundResource(const String &filepath);
	~SoundResource();

protected:
	virtual bool loadResourceImpl() override;

};

TS_END_PACKAGE1()
