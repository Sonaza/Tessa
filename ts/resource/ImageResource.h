#pragma once

#include "ts/resource/ResourceBase.h"

#include "SFML/Graphics/Image.hpp"

TS_PACKAGE1(resource)

class ImageResource : public resource::ResourceBase<sf::Image, TS_FOURCC('i','m','g','r')>
{
	TS_DECLARE_RESOURCE_TYPE(resource::ImageResource);

public:
	ImageResource(const String &filepath);
	~ImageResource();

protected:
	virtual bool loadResourceImpl() override;

};

TS_END_PACKAGE1()
