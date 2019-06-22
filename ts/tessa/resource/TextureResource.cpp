#include "Precompiled.h"
#include "ts/tessa/resource/TextureResource.h"

TS_DEFINE_RESOURCE_TYPE(resource::TextureResource);

TS_PACKAGE1(resource)

TextureResource::TextureResource(const std::string &filepath)
	: ResourceBase(filepath)
{
}

TextureResource::~TextureResource()
{
}

void TextureResource::setSmooth(const bool enabledParam)
{
	if (resource != nullptr)
		return;

	resource->setSmooth(enabledParam);
}

bool TextureResource::loadResourceImpl()
{
	if (!resource->loadFromFile(filepath))
	{
		TS_LOG_ERROR("Failed to load texture from file: %s", filepath.c_str());
		return false;
	}

	resource->setSmooth(true);
	return true;
}

TS_END_PACKAGE1()
