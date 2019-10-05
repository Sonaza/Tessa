#pragma once

#include "ts/resource/ResourceBase.h"

#include "SFML/Graphics/Texture.hpp"

TS_PACKAGE1(resource)

class TextureResource : public resource::ResourceBase<sf::Texture, TS_FOURCC('t','e','x','r')>
{
	TS_DECLARE_RESOURCE_TYPE(resource::TextureResource);

public:
	TextureResource(const String &filepath);
	~TextureResource();

	void setSmooth(const bool enabledParam);

protected:
	virtual bool loadResourceImpl() override;

};

TS_END_PACKAGE1()
