#pragma once

#include "ts/tessa/resource/GUID.h"

#include <queue>
#include <unordered_map>

TS_DECLARE1(system, Application)

TS_DECLARE1(resource, AbstractResourceBase)
TS_DECLARE1(resource, TextureResource)
TS_DECLARE1(resource, FontResource)
// TS_DECLARE1(resource, ShaderResource)
// TS_DECLARE1(resource, SoundResource)

TS_PACKAGE1(resource)

class ResourceManager
{
public:
	ResourceManager(std::shared_ptr<system::Application> application);
	~ResourceManager();

	void unloadAll();

	template<class ResourceType>
	std::shared_ptr<ResourceType> loadResource(const std::string &resourceHandle, const std::string &filepath);

	template<class ResourceType>
	std::shared_ptr<ResourceType> getResource(const std::string &resourceHandle);

	template<class ResourceType>
	std::shared_ptr<ResourceType> getResource(const GUID &resourceGuid);

	template<class ResourceType>
	std::shared_ptr<ResourceType> getResourceByFileGuid(const GUID &fileGuid);

private:
	void addResourceToLoadQueue(std::shared_ptr<AbstractResourceBase> resource);

	GUID findFileGuid(const GUID &resourceGuid);

	// Separate map to match resource guids to file guids
	typedef std::unordered_map<const GUID, GUID, GuidHash> ResourceGuidList;
	ResourceGuidList resourceGuids;

	// Abstract resource list holds any type of resource
	struct AbstractResource
	{
		std::shared_ptr<AbstractResourceBase> resource;
		SizeType typeId;
	};
	typedef std::unordered_map<const GUID, AbstractResource, GuidHash> AbstractResourceList;
	AbstractResourceList resources;

	// Typed resource list is collection of all types of resources, separated into their own buckets according to resource TypeId
// 	typedef std::unordered_map<SizeType, AbstractResourceList> TypedResourceList;
// 
// 	TypedResourceList resources;

	std::shared_ptr<system::Application> application;
};

template<class ResourceType>
std::shared_ptr<ResourceType> ResourceManager::loadResource(const std::string &resourceHandle, const std::string &filepath)
{
	std::shared_ptr<ResourceType> resource;

	GUID resourceGuid(resourceHandle);
	resource = getResource<ResourceType>(resourceGuid);
	if (resource != nullptr)
		return resource;

	GUID fileGuid(filepath);
	resource = getResourceByFileGuid<ResourceType>(fileGuid);
	if (resource != nullptr)
	{
		TS_PRINTF("Warning: Duplicate loading of resource '%s' detected, same file has been loaded with another GUID.\n", filepath.c_str());
		resourceGuids.emplace(resourceGuid, fileGuid);
		return resource;
	}

	resource = std::make_shared<ResourceType>(filepath);
	if (resource == nullptr)
	{
		TS_PRINTF("Allocating memory for resource failed.");
		return nullptr;
	}

	addResourceToLoadQueue(resource);

	AbstractResource ar;
	ar.resource = resource;
	ar.typeId = ResourceType::TypeId;

	resourceGuids.emplace(resourceGuid, fileGuid);
	resources.emplace(fileGuid, std::move(ar));

	return resource;
}

template<class ResourceType>
std::shared_ptr<ResourceType> ResourceManager::getResource(const std::string &resourceHandle)
{
	return getResource<ResourceType>(GUID(resourceHandle));
}

template<class ResourceType>
std::shared_ptr<ResourceType> ResourceManager::getResource(const GUID &resourceGuid)
{
	ResourceGuidList::iterator iter = resourceGuids.find(resourceGuid);
	if (iter == resourceGuids.end())
		return nullptr;

	AbstractResource &ar = resources[iter->second];
	if (ar.typeId == ResourceType::TypeId)
		return std::static_pointer_cast<ResourceType>(ar.resource);

	return nullptr;
}

template<class ResourceType>
std::shared_ptr<ResourceType> ResourceManager::getResourceByFileGuid(const GUID &fileGuid)
{
	AbstractResourceList::iterator iter = resources.find(fileGuid);
	if (iter != resources.end())
	{
		AbstractResource &ar = iter->second;
		if (ar.typeId == ResourceType::TypeId)
			return std::static_pointer_cast<ResourceType>(ar.resource);
	}
	return nullptr;
}


TS_END_PACKAGE1()
