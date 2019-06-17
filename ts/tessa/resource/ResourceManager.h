#pragma once

#include "ts/tessa/system/SystemManagerBase.h"
#include "ts/tessa/resource/AbstractResourceBase.h"
#include "ts/tessa/resource/GUID.h"

#include <queue>
#include <unordered_map>

TS_DECLARE1(system, Application)

TS_DECLARE1(resource, TextureResource)
TS_DECLARE1(resource, FontResource)
// TS_DECLARE1(resource, ShaderResource)
// TS_DECLARE1(resource, SoundResource)

TS_PACKAGE1(resource)

class ResourceManager : public system::SystemManagerBase<TS_FOURCC('R','M','A','N')>
{
	TS_DECLARE_SYSTEM_MANAGER_TYPE(resource::ResourceManager);

public:
	ResourceManager(system::Application *application);
	virtual ~ResourceManager();

	virtual bool initialize();
	virtual void deinitialize();

	virtual void update(const sf::Time deltaTime);

	template<class ResourceType>
	ResourceType *loadResource(const std::string &uniqueResourceHandle, const std::string &filepath, const bool immediate = false);

	template<class ResourceType>
	ResourceType *getResource(const std::string &uniqueResourceHandle);

	template<class ResourceType>
	ResourceType *getResource(const GUID &resourceGuid);

	template<class ResourceType>
	ResourceType *getResourceByFileGuid(const GUID &fileGuid);

	void unloadAll();

	bool unloadResource(const std::string &uniqueResourceHandle);
	bool unloadResource(const GUID &resourceGuid);
	bool unloadResourceByFilePath(const std::string &filepath);
	bool unloadResourceByFileGuid(const GUID &fileGuid);

private:
	static void loadResourceTask(AbstractResourceBase *resource);
	void addResourceToLoadQueue(AbstractResourceBase *resource);

	GUID findFileGuid(const GUID &resourceGuid);

	// Separate map to match resource guids to file guids
	typedef std::unordered_map<const GUID, GUID, GuidHash> GuidList;
	GuidList resourceGuids;

#if TS_BUILD != TS_FINALRELEASE
	std::map<const GUID, std::string> debugResourceHandles;
	std::map<SizeType, std::string> debugResourceTypeNames;
#endif

	// Abstract resource list holds any type of resource
	struct AbstractResourceContainer
	{
		AbstractResourceContainer(AbstractResourceBase *resource, SizeType typeId);
		~AbstractResourceContainer();
		bool isValid() const;

		UniquePointer<AbstractResourceBase> resource;
		SizeType typeId = ~0U;
	};
	typedef std::unordered_map<const GUID, UniquePointer<AbstractResourceContainer>, GuidHash> ResourceList;
	ResourceList resources;
};

template<class ResourceType>
ResourceType *ResourceManager::loadResource(const std::string &uniqueResourceHandle, const std::string &filepath, const bool immediate)
{
	ResourceType *resource = nullptr;

	GUID resourceGuid(uniqueResourceHandle);
	resource = getResource<ResourceType>(resourceGuid);
	if (resource != nullptr)
		return resource;

	GUID fileGuid(filepath);
	resource = getResourceByFileGuid<ResourceType>(fileGuid);
	if (resource != nullptr)
	{
#if TS_BUILD != TS_FINALRELEASE
		const std::string &debugHandle = debugResourceHandles[fileGuid];
		const GUID debugGUID(debugHandle);
		TS_ASSERTF(false, "Duplicate loading of a resource file, already loaded using a different resource handle.\n\nResource file: %s\nExisting handle: %s %s", filepath, debugHandle, debugGUID.getString());
#endif
		return nullptr;
	}

	resource = new(std::nothrow) ResourceType(filepath);
	if (resource == nullptr)
	{
		TS_LOG_ERROR("Allocating resource failed.");
		return nullptr;
	}
	
	UniquePointer<AbstractResourceContainer> rc = makeUnique<AbstractResourceContainer>(resource, ResourceType::TypeId);
	if (rc == nullptr)
	{
		TS_LOG_ERROR("Allocating AbstractResourceContainer failed.");
		return nullptr;
	}

	resourceGuids.emplace(resourceGuid, fileGuid);
	resources.emplace(fileGuid, std::move(rc));

#if TS_BUILD != TS_FINALRELEASE
	debugResourceHandles[fileGuid] = uniqueResourceHandle;
	debugResourceTypeNames[ResourceType::TypeId] = ResourceType::TypeName;
#endif

	// Add to threaded load queue or load immediately based on user flag
	if (immediate == false)
	{
		addResourceToLoadQueue(resource);
	}
	else
	{
		loadResourceTask(resource);
	}

	return resource;
}

template<class ResourceType>
ResourceType *ResourceManager::getResource(const std::string &uniqueResourceHandle)
{
	return getResource<ResourceType>(GUID(uniqueResourceHandle));
}

template<class ResourceType>
ResourceType *ResourceManager::getResource(const GUID &resourceGuid)
{
	GuidList::iterator iter = resourceGuids.find(resourceGuid);
	if (iter == resourceGuids.end())
		return nullptr;

	return getResourceByFileGuid<ResourceType>(iter->second);
}

template<class ResourceType>
ResourceType *ResourceManager::getResourceByFileGuid(const GUID &fileGuid)
{
	ResourceList::iterator iter = resources.find(fileGuid);
	if (iter != resources.end())
	{
#if TS_BUILD != TS_FINALRELEASE
		TS_ASSERTF(iter->second != nullptr && iter->second->isValid(), "AbstractResourceContainer is nullptr or invalid.");
		const std::string &debugHandle = debugResourceHandles[fileGuid];
		const GUID debugGUID(debugHandle);
		TS_ASSERTF(iter->second->typeId == ResourceType::TypeId,
			"Resource types don't match, requested resource handle is already used by a different resource type.\n\nResource handle: %s %s\nExisting type: %s\nRequested type: %s",
			debugHandle, debugGUID.getString(), debugResourceTypeNames[iter->second->typeId], ResourceType::TypeName
		);
#endif
		if (iter->second != nullptr && iter->second->isValid() && iter->second->typeId == ResourceType::TypeId)
			return static_cast<ResourceType*>(iter->second->resource.get());
	}
	return nullptr;
}


TS_END_PACKAGE1()
