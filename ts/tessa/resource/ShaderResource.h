#pragma once

#include "ts/tessa/resource/ResourceBase.h"

TS_PACKAGE1(resource)

class ShaderResource : public ResourceBase<sf::Shader>
{
public:
	ShaderResource(const std::string &filepath);
	~ShaderResource();

	bool loadResource();

};

TS_END_PACKAGE1()
