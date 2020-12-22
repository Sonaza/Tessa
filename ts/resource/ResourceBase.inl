

template <class ResourceType, SizeType ResourceTypeIndex>
ts::resource::ResourceBase<ResourceType, ResourceTypeIndex>::ResourceBase()
	: AbstractResourceBase(DataSourceGenerated)
	, resourceGuid(generateRandomGuid())
{
}

template <class ResourceType, SizeType ResourceTypeIndex>
ts::resource::ResourceBase<ResourceType, ResourceTypeIndex>::ResourceBase(const String &filepath)
	: AbstractResourceBase(DataSourceFile, filepath)
	, resourceGuid(filepath)
{
	TS_ASSERT(!filepath.isEmpty() && "Resource filepath is not set.");
}

template <class ResourceType, SizeType ResourceTypeIndex>
ts::resource::ResourceBase<ResourceType, ResourceTypeIndex>::~ResourceBase()
{
	unloadResource();
}

template <class ResourceType, SizeType ResourceTypeIndex>
bool ResourceBase<ResourceType, ResourceTypeIndex>::loadResource()
{
	if (isLoaded())
		return false;

	MutexGuard lock(resourceMutex);

	resourceLoaded = false;

	resource = makeShared<ResourceType>();
	if (resource == nullptr)
	{
		TS_LOG_ERROR("Failed to allocate memory for resource container.");
		loadError = true;
		return false;
	}

	bool success = loadResourceImpl();
	if (!success)
	{
		TS_LOG_ERROR("LoadResourceImpl failed.");
		loadError = true;
		return false;
	}

	resourceLoaded = true;
	loadError = false;
	return true;
}

template <class ResourceType, SizeType ResourceTypeIndex>
void ResourceBase<ResourceType, ResourceTypeIndex>::unloadResource()
{
	MutexGuard lock(resourceMutex);
	resource.reset();
	resourceLoaded = false;
	loadError = false;
}

template <class ResourceType, SizeType ResourceTypeIndex>
SharedPointer<ResourceType> ResourceBase<ResourceType, ResourceTypeIndex>::getResource() const
{
	TS_ASSERT(resourceLoaded && "Resource is not loaded");
	MutexGuard lock(resourceMutex);
	return resource;
}
