#include "Precompiled.h"
#include "ts/tessa/resource/ResourceManager.h"

#include "ts/tessa/system/Application.h"
#include "ts/tessa/system/ThreadPool.h"

#include "ts/tessa/resource/TextureResource.h"
#include "ts/tessa/resource/FontResource.h"
// #include "ts/tessa/resource/ShaderResource.h"
// #include "ts/tessa/resource/SoundResource.h"

TS_DEFINE_SYSTEM_MANAGER_TYPE(resource::ResourceManager);

TS_PACKAGE1(resource)

ResourceManager::ResourceManager(system::Application *application)
	: SystemManagerBase(application)
{
}

ResourceManager::~ResourceManager()
{
	unloadAll();
}

bool ResourceManager::initialize()
{
	return true;
}

void ResourceManager::deinitialize()
{
	unloadAll();
}

void ResourceManager::unloadAll()
{
	for (ResourceList::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		AbstractResourceContainer *rc = it->second;
		if (rc != nullptr)
			delete it->second;
	}
	resources.clear();
}

bool ResourceManager::unloadResource(const std::string &uniqueResourceHandle)
{
	return unloadResource(GUID(uniqueResourceHandle));
}

bool ResourceManager::unloadResource(const GUID &resourceGuid)
{
	GuidList::iterator guidIter = resourceGuids.find(resourceGuid);
	if (guidIter == resourceGuids.end())
		return false;

	ResourceList::iterator resourceIter = resources.find(guidIter->second);
	if (resourceIter == resources.end())
		return false;

	resourceGuids.erase(guidIter);

	AbstractResourceContainer *rc = resourceIter->second;
	TS_ASSERT(rc != nullptr && "AbstractResourceContainer is nullptr.");

	if (rc != nullptr && rc->numReferences > 1)
	{
		rc->numReferences--;
	}
	else
	{
		if (rc != nullptr)
			delete rc;
		resources.erase(resourceIter);
	}
	return true;
}

bool ResourceManager::unloadResourceByFilePath(const std::string &filepath)
{
	return unloadResourceByFileGuid(GUID(filepath));
}

bool ResourceManager::unloadResourceByFileGuid(const GUID &fileGuid)
{
	ResourceList::iterator resourceIter = resources.find(fileGuid);
	if (resourceIter == resources.end())
		return false;

	for (GuidList::iterator guidIter = resourceGuids.begin(); guidIter != resourceGuids.end();)
	{
		if (guidIter->second == fileGuid)
			guidIter = resourceGuids.erase(guidIter);
		else
			++guidIter;
	}

	AbstractResourceContainer *rc = resourceIter->second;
	TS_ASSERT(rc != nullptr && "AbstractResourceContainer is nullptr.");

	if (rc != nullptr)
		delete rc;
	resources.erase(resourceIter);

	return true;
}

void ResourceManager::addResourceToLoadQueue(AbstractResourceBase *resource)
{
	TS_ASSERT(resource != nullptr);
	TS_VERIFY_POINTERS(application, resource);

	application->threadPool->push(system::ThreadPool::Normal, [resource]()
	{
		TS_ASSERT(resource != nullptr);
		TS_LOG_DEBUG("Now loading resource 0x%08X", (ptrdiff_t)resource);
		resource->loadResource();
	});
}

GUID ResourceManager::findFileGuid(const GUID &resourceGuid)
{
	GuidList::iterator iter = resourceGuids.find(resourceGuid);
	if (iter != resourceGuids.end())
		return iter->second;
	return GUID::none;
}

ResourceManager::AbstractResourceContainer::AbstractResourceContainer(AbstractResourceBase *resource, SizeType typeId)
	: resource(resource)
	, typeId(typeId)
	, numReferences(1)
{
}

ResourceManager::AbstractResourceContainer::~AbstractResourceContainer()
{
	if (resource != nullptr)
	{
		TS_PRINTF("Unloading resource 0x%" PRIXPTR " with typeid %u\n", (ptrdiff_t)resource, typeId);

		resource->unloadResource();
		delete resource;
	}
	resource = nullptr;
}

bool ResourceManager::AbstractResourceContainer::isValid() const
{
	return resource != nullptr && typeId != ~0U && numReferences > 0;
}

TS_END_PACKAGE1()

