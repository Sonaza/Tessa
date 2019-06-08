#pragma once

#include "ts/tessa/resource/AbstractResourceBase.h"
#include "ts/tessa/resource/GUID.h"

#include <atomic>
#include <mutex>

TS_PACKAGE1(resource)

template <class ResourceType, SizeType ResourceTypeIndex>
class ResourceBase : public AbstractResourceBase
{
public:
	enum { TypeId = ResourceTypeIndex };

	explicit ResourceBase(const std::string &filepath);
	virtual ~ResourceBase();

	const std::string &getFilepath() const { return filepath; }
	GUID getGuid() const { return resourceGuid; }

	bool isLoaded() const { return resourceLoaded; }
	bool hasError() const { return loadError; }

	virtual bool loadResource();
	virtual void unloadResource();

	ResourceType *getResource() const;

protected:
	virtual bool loadResourceImpl() = 0;

	std::unique_ptr<ResourceType> resource;

	std::atomic<bool> resourceLoaded = false;
	std::atomic<bool> loadError = false;

	std::string filepath;
	GUID resourceGuid;

	mutable std::mutex resourceMutex;
};

#include "ResourceBase.inl"

TS_END_PACKAGE1()
