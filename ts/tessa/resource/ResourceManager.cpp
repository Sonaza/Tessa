#include "Precompiled.h"
#include "ts/tessa/resource/ResourceManager.h"

#include "ts/tessa/system/BaseApplication.h"
#include "ts/tessa/threading/ThreadScheduler.h"

#include "ts/tessa/resource/TextureResource.h"
#include "ts/tessa/resource/FontResource.h"
#include "ts/tessa/resource/SoundResource.h"
#include "ts/tessa/resource/MusicResource.h"
// #include "ts/tessa/resource/ShaderResource.h"

TS_DEFINE_SYSTEM_MANAGER_TYPE(resource::ResourceManager);

TS_PACKAGE1(resource)

std::string ResourceManager::resourceRootDirectory = "";

std::atomic_bool ResourceManager::stop_flag;

ResourceManager::ResourceManager(system::BaseApplication *application)
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
	ResourceManager::stop_flag.store(true);
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

void ResourceManager::setResourceRootDirectory(const std::string &rootDirectory)
{
	ResourceManager::resourceRootDirectory = rootDirectory;
}

const std::string &ResourceManager::getResourceRootDirectory()
{
	return ResourceManager::resourceRootDirectory;
}

void ResourceManager::loadResourceTask(SharedPointer<AbstractResourceBase> resource)
{
	if (ResourceManager::stop_flag.load(std::memory_order_relaxed))
		return;

	// If resource is unique that means all other references are already gone and there is no point in loading.
	if (resource.isUnique())
	{
		TS_PRINTF("skedaddle\n");
		return;
	}

	TS_ASSERT(resource != nullptr);
// 	TS_LOG_DEBUG("Now loading resource 0x%016X", (ptrdiff_t)resource.get());
	resource->loadResource();
}

void ResourceManager::addResourceToLoadQueue(SharedPointer<AbstractResourceBase> resource)
{
	TS_ASSERT(resource != nullptr);
	TS_VERIFY_POINTERS(application, resource);

// 	if (ResourceManager::stop_flag.load(std::memory_order_relaxed))
// 		return;

	threading::ThreadScheduler &tm = getGigaton<threading::ThreadScheduler>();
	tm.scheduleOnce(0, &ResourceManager::loadResourceTask, resource);
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
// 		TS_PRINTF("Unloading resource 0x%" PRIXPTR " with typeid %u\n", (ptrdiff_t)resource.get(), typeId);
		resource->unloadResource();
	}
	resource.reset();
}

bool ResourceManager::AbstractResourceContainer::isValid() const
{
	return resource != nullptr && typeId != ~0U;
}

TS_END_PACKAGE1()

