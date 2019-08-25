#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"
#include "ts/tessa/resource/AbstractResourceBase.h"
#include "ts/tessa/resource/GUID.h"

#include <queue>
#include <unordered_map>
#include <atomic>

TS_DECLARE1(system, BaseApplication);

TS_DECLARE1(resource, TextureResource);
TS_DECLARE1(resource, FontResource);
TS_DECLARE1(resource, ShaderResource);
TS_DECLARE1(resource, MusicResource);
TS_DECLARE1(resource, SoundResource);

TS_PACKAGE1(resource)

class ResourceManager : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(resource::ResourceManager);

public:
	ResourceManager();
	virtual ~ResourceManager();

	virtual bool initialize();
	virtual void deinitialize();

	virtual void update(const TimeSpan deltaTime);

	template<class ResourceType>
	ResourceType *loadResource(const std::string &uniqueResourceHandle, const std::string &filepath, const bool immediate = false);

	template<class ResourceType>
	ResourceType *getResource(const std::string &uniqueResourceHandle) const;

	template<class ResourceType>
	ResourceType *getResource(const GUID &resourceGuid) const;

	template<class ResourceType>
	ResourceType *getResourceByFileGuid(const GUID &fileGuid) const;

	TextureResource *loadTexture(const std::string &uniqueResourceHandle, const std::string &filepath, const bool immediate = false);
	FontResource *loadFont(const std::string &uniqueResourceHandle, const std::string &filepath, const bool immediate = false);
	ShaderResource *loadShader(const std::string &uniqueResourceHandle, const std::string &filepath, const bool immediate = false);
	MusicResource *loadMusic(const std::string &uniqueResourceHandle, const std::string &filepath, const bool immediate = false);
	SoundResource *loadSound(const std::string &uniqueResourceHandle, const std::string &filepath, const bool immediate = false);

	TextureResource *getTexture(const std::string &uniqueResourceHandle) const;
	FontResource *getFont(const std::string &uniqueResourceHandle) const;
	ShaderResource *getShader(const std::string &uniqueResourceHandle) const;
	MusicResource *getMusic(const std::string &uniqueResourceHandle) const;
	SoundResource *getSound(const std::string &uniqueResourceHandle) const;

	void unloadAll();

	bool unloadResource(const std::string &uniqueResourceHandle);
	bool unloadResource(const GUID &resourceGuid);
	bool unloadResourceByFilePath(const std::string &filepath);
	bool unloadResourceByFileGuid(const GUID &fileGuid);

	// Change the resource root directory, by default it uses working directory
	// through releative paths but that may not always be correct.
	static void setResourceRootDirectory(const std::string &rootDirectory);
	static const std::string &getResourceRootDirectory();

	// Converts the given filepath to an absolute resource path, relative to the resource root.
	static std::string getAbsoluteResourcePath(const std::string &filepath);

private:
	static std::atomic_bool stop_flag;
	static std::string resourceRootDirectory;

	static void loadResourceTask(SharedPointer<AbstractResourceBase> resource);
	void addResourceToLoadQueue(SharedPointer<AbstractResourceBase> resource);

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

		SharedPointer<AbstractResourceBase> resource;
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

#if TS_BUILD != TS_FINALRELEASE
	debugResourceHandles[fileGuid] = uniqueResourceHandle;
	debugResourceTypeNames[ResourceType::TypeId] = ResourceType::TypeName;
#endif

	// Add to threaded load queue or load immediately based on user flag
	if (immediate == false)
	{
		addResourceToLoadQueue(rc->resource);
	}
	else
	{
		loadResourceTask(rc->resource);
	}

	resourceGuids.emplace(resourceGuid, fileGuid);
	resources.emplace(fileGuid, std::move(rc));

	return resource;
}

template<class ResourceType>
ResourceType *ResourceManager::getResource(const std::string &uniqueResourceHandle) const
{
	return getResource<ResourceType>(GUID(uniqueResourceHandle));
}

template<class ResourceType>
ResourceType *ResourceManager::getResource(const GUID &resourceGuid) const
{
	GuidList::const_iterator iter = resourceGuids.find(resourceGuid);
	if (iter == resourceGuids.end())
		return nullptr;

	return getResourceByFileGuid<ResourceType>(iter->second);
}

template<class ResourceType>
ResourceType *ResourceManager::getResourceByFileGuid(const GUID &fileGuid) const
{
	ResourceList::const_iterator iter = resources.find(fileGuid);
	if (iter != resources.end())
	{
		const UniquePointer<AbstractResourceContainer> &rc = iter->second;

#if TS_BUILD != TS_FINALRELEASE
		TS_ASSERTF(rc != nullptr && rc->isValid(), "AbstractResourceContainer is nullptr or invalid.");
		const std::string &debugHandle = debugResourceHandles.at(fileGuid);
		const GUID debugGUID(debugHandle);
		TS_ASSERTF(rc->typeId == ResourceType::TypeId,
			"Resource types don't match, requested resource handle is already used by a different resource type.\n\n"
			"Resource handle: %s %s\nExisting type: %s\nRequested type: %s",
			debugHandle, debugGUID.getString(), debugResourceTypeNames.at(rc->typeId), ResourceType::TypeName
			);
#endif
		if (rc != nullptr && rc->isValid() && rc->typeId == ResourceType::TypeId)
			return static_cast<ResourceType*>(rc->resource.get());
	}
	return nullptr;
}


TS_END_PACKAGE1()
