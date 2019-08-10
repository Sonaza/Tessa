#pragma once

#include "ts/tessa/resource/AbstractResourceBase.h"
#include "ts/tessa/resource/GUID.h"

#include <atomic>
#include <mutex>

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

	UniquePointer<ResourceType> resource;

	std::atomic<bool> resourceLoaded = false;
	std::atomic<bool> loadError = false;

	GUID resourceGuid;

	mutable std::mutex resourceMutex;
};

#include "ResourceBase.inl"

TS_END_PACKAGE1()
