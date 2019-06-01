#pragma once

#include "ts/tessa/resource/ResourceBase.h"

TS_PACKAGE1(resource)

class TextureResource : public ResourceBase<sf::Texture>
{
public:
	static const SizeType TypeId = FOUR_CC('t','e','x','r');

	TextureResource(const std::string &filepath);
	~TextureResource();

	void setSmooth(const bool enabledParam);

protected:
	virtual bool loadResourceImpl() override;

};

TS_END_PACKAGE1()
