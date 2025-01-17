#pragma once

#include "ts/resource/ResourceBase.h"

#include "SFML/Graphics/Shader.hpp"

TS_PACKAGE1(resource)

class ShaderResource : public resource::ResourceBase<sf::Shader, TS_FOURCC('s','h','d','r')>
{
	TS_DECLARE_RESOURCE_TYPE(resource::ShaderResource);

public:
	ShaderResource(const String &filepath);
	~ShaderResource();

protected:
	virtual bool loadResourceImpl() override;

};

TS_END_PACKAGE1()
