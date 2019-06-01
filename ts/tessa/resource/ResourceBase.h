#pragma once

#include "ts/tessa/resource/GUID.h"

#include <atomic>
#include <mutex>

TS_PACKAGE1(resource)

class AbstractResourceBase
{
public:
	static const SizeType TypeId = FOUR_CC('a','r','s','b');

	AbstractResourceBase() {}
	virtual ~AbstractResourceBase() {}

	virtual bool loadResource() = 0;
	virtual void unloadResource() = 0;
};

template <class ResourceType>
class ResourceBase : public AbstractResourceBase
{
public:
	static const SizeType TypeId = FOUR_CC('r','e','s','b');

	explicit ResourceBase(const std::string &filepath)
		: resourceGuid(filepath)
		, filepath(filepath)
	{
		TS_ASSERT(!filepath.empty() && "Resource filepath is not set.");
	}

	virtual ~ResourceBase()
	{
		unloadResource();
	}

	const std::string &getFilepath() const { return filepath; }

	bool isLoaded() const { return resourceLoaded; }
	bool hasError() const { return loadError; }

	virtual bool loadResource();

	void unloadResource()
	{
		std::lock_guard<std::mutex> lock(resourceMutex);
		resource.reset();
		resourceLoaded = false;
		loadError = false;
	}

	std::shared_ptr<ResourceType> getResource() const
	{
		TS_ASSERT(resourceLoaded && "Resource is not loaded");
		std::lock_guard<std::mutex> lock(resourceMutex);
		return resource;
	}

	GUID getGuid() const
	{
		return resourceGuid;
	}

protected:
	virtual bool loadResourceImpl() = 0;

	std::shared_ptr<ResourceType> resource;

	std::atomic<bool> resourceLoaded = false;
	std::atomic<bool> loadError = false;

	std::string filepath;
	GUID resourceGuid;

	mutable std::mutex resourceMutex;
};

template <class ResourceType>
bool ResourceBase<ResourceType>::loadResource()
{
	if (isLoaded())
		return false;

	std::lock_guard<std::mutex> lock(resourceMutex);

	resourceLoaded = false;

	resource = std::make_shared<ResourceType>();
	if (resource == nullptr)
	{
		TS_PRINTF("Failed to allocate memory for resource container.");
		loadError = true;
		return false;
	}

	bool success = loadResourceImpl();
	if (!success)
	{
		
		loadError = true;
		return false;
	}

	resourceLoaded = true;
	loadError = false;
	return true;
}

TS_END_PACKAGE1()
