#pragma once

#include "ts/tessa/resource/AbstractResourceBase.h"
#include "ts/tessa/resource/GUID.h"
#include "ts/tessa/thread/Mutex.h"
#include "ts/tessa/thread/MutexGuard.h"

#include <atomic>

#define TS_DECLARE_RESOURCE_TYPE(__class_name) \
	public: \
		static const char * TypeName;

#define TS_DEFINE_RESOURCE_TYPE(__class_name) \
	namespace ts { const char * __class_name::TypeName = #__class_name; }

TS_PACKAGE1(resource)

template <class ResourceType, SizeType ResourceTypeIndex>
class ResourceBase : public AbstractResourceBase
{
public:
	enum { TypeId = ResourceTypeIndex };

	explicit ResourceBase(const String &filepath);
	virtual ~ResourceBase();

	const String &getFilepath() const { return filepath; }
	GUID getGuid() const { return resourceGuid; }

	bool isLoaded() const { return resourceLoaded; }
	bool hasError() const { return loadError; }

	virtual bool loadResource();
	virtual void unloadResource();

	bool reloadResource()
	{
		unloadResource();
		return loadResource();
	}

	SharedPointer<ResourceType> getResource() const;

protected:
	virtual bool loadResourceImpl() = 0;

	SharedPointer<ResourceType> resource;

	std::atomic<bool> resourceLoaded = false;
	std::atomic<bool> loadError = false;

	GUID resourceGuid;

	mutable Mutex resourceMutex;
};

#include "ResourceBase.inl"

TS_END_PACKAGE1()
