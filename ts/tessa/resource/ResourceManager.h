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

	template<class ResourceType>
	ResourceType *loadResource(const std::string &uniqueResourceHandle, const std::string &filepath);

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
	void addResourceToLoadQueue(AbstractResourceBase *resource);

	GUID findFileGuid(const GUID &resourceGuid);

	// Separate map to match resource guids to file guids
	typedef std::unordered_map<const GUID, GUID, GuidHash> GuidList;
	GuidList resourceGuids;

	// Abstract resource list holds any type of resource
	struct AbstractResourceContainer
	{
		AbstractResourceContainer(AbstractResourceBase *resource, SizeType typeId);
		~AbstractResourceContainer();
		bool isValid() const;

		AbstractResourceBase *resource = nullptr;
		SizeType typeId = ~0U;
		SizeType numReferences = 0;
	};
	typedef std::unordered_map<const GUID, AbstractResourceContainer*, GuidHash> ResourceList;
	ResourceList resources;
};

template<class ResourceType>
ResourceType *ResourceManager::loadResource(const std::string &uniqueResourceHandle, const std::string &filepath)
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
		TS_LOG_WARNING("Duplicate loading of resource '%s' detected, same file has been loaded with another GUID.", filepath.c_str());
		resourceGuids.emplace(resourceGuid, fileGuid);
		resources[fileGuid]->numReferences++;
		return resource;
	}

	resource = new(std::nothrow) ResourceType(filepath);
	if (resource == nullptr)
	{
		TS_LOG_ERROR("Allocating resource failed.");
		return nullptr;
	}

	TS_PRINTF("1 resource 0x%08X\n", (ptrdiff_t)resource);

	AbstractResourceContainer *rc = new AbstractResourceContainer(resource, ResourceType::TypeId);
	if (rc == nullptr)
	{
		TS_LOG_ERROR("Allocating AbstractResourceContainer failed.");
		return nullptr;
	}

	TS_PRINTF("2 resource 0x%08X\n", (ptrdiff_t)resource);

	resourceGuids.emplace(resourceGuid, fileGuid);
	resources.emplace(fileGuid, rc);

	TS_PRINTF("3 resource 0x%08X\n", (ptrdiff_t)resource);

	TS_LOG_DEBUG("Adding resource to threaded load queue.");
	addResourceToLoadQueue(resource);

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
		AbstractResourceContainer *rc = iter->second;
		TS_ASSERT(rc != nullptr && "AbstractResourceContainer is nullptr.");
		TS_ASSERT(rc->isValid() && "AbstractResourceContainer is not valid. Resource might not have been set.");
		TS_ASSERT(rc->typeId == ResourceType::TypeId && "Resource types don't match, same resource handle is already being used by different resource type.");
		if (rc != nullptr && rc->isValid() && rc->typeId == ResourceType::TypeId)
			return static_cast<ResourceType*>(rc->resource);
	}
	return nullptr;
}


TS_END_PACKAGE1()
