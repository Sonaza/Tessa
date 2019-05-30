#include "Precompiled.h"
#include "ts/tessa/resource/ResourceManager.h"

#include "ts/tessa/resource/TextureResource.h"
// #include "ts/tessa/resource/FontResource.h"
// #include "ts/tessa/resource/ShaderResource.h"
// #include "ts/tessa/resource/SoundResource.h"

TS_PACKAGE1(resource)

ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{
	unloadAll();
}

void ResourceManager::unloadAll()
{

}

std::shared_ptr<TextureResource> ResourceManager::loadTexture(const std::string &resourceHandle, const std::string &filepath)
{
	GUID resourceGuid(resourceHandle);
	
	GUID fileGuid = findFileGuid(resourceGuid);
	if (fileGuid != GUID::none)
		return textureResources[fileGuid];

// 	TextureResourceList::iterator iter = textureResources.find(fileGuid);
// 	if (iter != textureResources.end())
// 		return iter->second;

	std::shared_ptr<TextureResource> res = std::make_shared<TextureResource>(filepath);
	if (res == nullptr)
	{
		TS_PRINTF("Loading texture resource failed.");
		return nullptr;
	}

	if (!res->loadResource())
	{
		TS_PRINTF("Loading texture resource failed.");
		return nullptr;
	}

	resourceGuids.insert(std::make_pair(resourceGuid, res->getGuid()));
	textureResources.insert(std::make_pair(res->getGuid(), res));

	return res;
}

GUID ResourceManager::findFileGuid(const GUID &resourceGuid)
{
	ResourceGuidList::iterator iter = resourceGuids.find(resourceGuid);
	if (iter != resourceGuids.end())
		return iter->second;
	return GUID::none;
}

TS_END_PACKAGE1()

