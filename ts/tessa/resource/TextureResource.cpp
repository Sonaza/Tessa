#include "Precompiled.h"
#include "ts/tessa/resource/TextureResource.h"

TS_PACKAGE1(resource)

TextureResource::TextureResource(const std::string &filepath)
	: ResourceBase(filepath)
{
}

TextureResource::~TextureResource()
{
}

bool TextureResource::loadResource()
{
	TS_ASSERT(!isLoaded() && "Texture resource has already been loaded.");
	if (isLoaded())
		return false;

	resource = std::make_shared<sf::Texture>();
	if (resource == nullptr)
	{
		TS_PRINTF("Failed to allocate texture.");
		loadError = true;
		return false;
	}

	if (!resource->loadFromFile(filepath))
	{
		TS_PRINTF("Failed to load texture from file: %s\n", filepath.c_str());
		loadError = true;
		return false;
	}
	
	resource->setSmooth(true);

	resourceLoaded = true;
	loadError = false;
	return true;
}

void TextureResource::setSmooth(const bool enabledParam)
{
	if (resource != nullptr)
		return;

	resource->setSmooth(enabledParam);
}

TS_END_PACKAGE1()
