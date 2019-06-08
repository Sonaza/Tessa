#include "Precompiled.h"
#include "ts/tessa/resource/FontResource.h"

TS_PACKAGE1(resource)

FontResource::FontResource(const std::string &filepath)
	: ResourceBase(filepath)
{
	
}

FontResource::~FontResource()
{
	
}

bool FontResource::loadResourceImpl()
{
	if (!resource->loadFromFile(filepath))
	{
		TS_LOG_ERROR("Failed to load font from file: %s\n", filepath.c_str());
		return false;
	}
	return true;
}

TS_END_PACKAGE1()
