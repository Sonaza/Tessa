#pragma once

#include "ts/tessa/resource/GUID.h"

TS_PACKAGE1(resource)

template <class ResourceType>
class ResourceBase
{
public:
	explicit ResourceBase(const std::string &filepath)
		: resourceGuid(filepath)
		, filepath(filepath)
	{
		TS_ASSERT(!filepath.empty() && "Resource filepath is not set.");
	}

	~ResourceBase()
	{
		unloadResource();
	}

	const std::string &getFilepath() const { return filepath; }

	bool isLoaded() const { return resourceLoaded; }
	bool hasError() const { return loadError; }

	virtual bool loadResource() = 0;

	void unloadResource()
	{
		resource.reset();
		resourceLoaded = false;
		loadError = false;
	}

	std::shared_ptr<ResourceType> getResource() const
	{
		return resource;
	}

	GUID getGuid() const
	{
		return resourceGuid;
	}

protected:
	std::shared_ptr<ResourceType> resource;

	bool resourceLoaded = false;
	bool loadError = false;

	std::string filepath;
	GUID resourceGuid;
};

TS_END_PACKAGE1()
