#pragma once

#include "ts/tessa/resource/GUID.h"

#include <unordered_map>

TS_DECLARE1(resource, TextureResource)
// TS_DECLARE1(resource, FontResource)
// TS_DECLARE1(resource, ShaderResource)
// TS_DECLARE1(resource, SoundResource)

TS_PACKAGE1(resource)

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void unloadAll();

	template<class ResourceType>
	std::shared_ptr<ResourceType> getResource(const std::string &resourceHandle);

	template<class ResourceType>
	std::shared_ptr<ResourceType> getResource(const GUID &resourceGuid);

	std::shared_ptr<TextureResource> loadTexture(const std::string &resourceHandle, const std::string &filepath);

private:
	GUID findFileGuid(const GUID &resourceGuid);

	// Separate map to match resource guids to file guids
	typedef std::unordered_map<const GUID, GUID, GuidHash> ResourceGuidList;
	ResourceGuidList resourceGuids;

	typedef std::unordered_map<const GUID, std::shared_ptr<TextureResource>, GuidHash> TextureResourceList;
	TextureResourceList textureResources;
};


// template<>
// std::shared_ptr<TextureResource> ResourceManager::getResource(const std::string &resourceHandle)
// {
// 	GUID resourceGuid(resourceHandle);
// 	GUID fileGuid = findFileGuid(resourceGuid);
// 
// 	TextureResourceList::iterator iter = textureResources.find(fileGuid);
// 	if (iter != textureResources.end())
// 		return iter->second;
// 
// 	return nullptr;
// }

template<class ResourceType>
std::shared_ptr<ResourceType> ResourceManager::getResource(const std::string &resourceHandle)
{
// 	static_assert(false, "Resource type query should have a custom template implementation.");
	GUID resourceGuid(resourceHandle);
// 	GUID fileGuid = findFileGuid(resourceGuid);
// 
// 	TextureResourceList::iterator iter = textureResources.find(fileGuid);
// 	if (iter != textureResources.end())
// 		return iter->second;

	GUID fileGuid = findFileGuid(resourceGuid);
	if (fileGuid != GUID::none)
		return textureResources[fileGuid];

	return nullptr;
}


TS_END_PACKAGE1()
