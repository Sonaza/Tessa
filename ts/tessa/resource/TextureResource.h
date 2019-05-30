#pragma once

#include "ts/tessa/resource/ResourceBase.h"

TS_PACKAGE1(resource)

class TextureResource : public ResourceBase<sf::Texture>
{
public:
	TextureResource(const std::string &filepath);
	~TextureResource();

	virtual bool loadResource() override;

	void setSmooth(const bool enabledParam);

};

TS_END_PACKAGE1()
