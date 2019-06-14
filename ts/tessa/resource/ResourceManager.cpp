#include "Precompiled.h"
#include "ts/tessa/resource/ResourceManager.h"

#include "ts/tessa/system/Application.h"
#include "ts/tessa/threading/ThreadManager.h"

#include "ts/tessa/resource/TextureResource.h"
#include "ts/tessa/resource/FontResource.h"
// #include "ts/tessa/resource/ShaderResource.h"
// #include "ts/tessa/resource/SoundResource.h"

TS_DEFINE_SYSTEM_MANAGER_TYPE(resource::ResourceManager);

TS_PACKAGE1(resource)

ResourceManager::ResourceManager(system::Application *application)
	: SystemManagerBase(application)
{
	TS_GIGATON_REGISTER_CLASS(this);
}

ResourceManager::~ResourceManager()
{
	TS_GIGATON_UNREGISTER_CLASS(this);
}

bool ResourceManager::initialize()
{
	return true;
}

void ResourceManager::deinitialize()
{
	unloadAll();
}

void ResourceManager::update(const sf::Time deltaTime)
{

}

void ResourceManager::unloadAll()
{
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
	resources.erase(resourceIter);

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

	resources.erase(resourceIter);

	return true;
}

void ResourceManager::loadResourceTask(AbstractResourceBase *resource)
{
	TS_ASSERT(resource != nullptr);
	TS_LOG_DEBUG("Now loading resource 0x%08X", (ptrdiff_t)resource);
	resource->loadResource();
}

void ResourceManager::addResourceToLoadQueue(AbstractResourceBase *resource)
{
	TS_ASSERT(resource != nullptr);
	TS_VERIFY_POINTERS(application, resource);

	threading::ThreadManager &tm = getGigaton<threading::ThreadManager>();
	tm.push(threading::TaskPriorityNormal, &ResourceManager::loadResourceTask, resource);
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
{
}

ResourceManager::AbstractResourceContainer::~AbstractResourceContainer()
{
	if (resource != nullptr)
	{
		TS_PRINTF("Unloading resource 0x%" PRIXPTR " with typeid %u\n", (ptrdiff_t)resource.get(), typeId);
		resource->unloadResource();
	}
	resource.reset();
}

bool ResourceManager::AbstractResourceContainer::isValid() const
{
	return resource != nullptr && typeId != ~0U;
}

TS_END_PACKAGE1()

