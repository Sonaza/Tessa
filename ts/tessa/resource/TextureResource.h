#pragma once

#include "ts/tessa/resource/ResourceBase.h"

TS_PACKAGE1(resource)

class TextureResource : public resource::ResourceBase<sf::Texture, TS_FOURCC('t','e','x','r')>
{
public:
	TextureResource(const std::string &filepath);
	~TextureResource();

	void setSmooth(const bool enabledParam);

protected:
	virtual bool loadResourceImpl() override;

};

TS_END_PACKAGE1()
