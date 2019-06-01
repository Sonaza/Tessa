#include "Precompiled.h"
#include "ts/tessa/resource/ResourceManager.h"

#include "ts/tessa/system/Application.h"
#include "ts/tessa/system/ThreadPool.h"

#include "ts/tessa/resource/TextureResource.h"
#include "ts/tessa/resource/FontResource.h"
// #include "ts/tessa/resource/ShaderResource.h"
// #include "ts/tessa/resource/SoundResource.h"

TS_PACKAGE1(resource)

ResourceManager::ResourceManager(std::shared_ptr<system::Application> application)
	: application(application)
{
}

ResourceManager::~ResourceManager()
{
	unloadAll();
}

void ResourceManager::unloadAll()
{
	resources.clear();
}

void ResourceManager::addResourceToLoadQueue(std::shared_ptr<AbstractResourceBase> resource)
{
	TS_VERIFY_POINTERS(application);

	application->threadPool->push(system::ThreadPool::Normal, [resource]()
	{
		if (resource != nullptr)
			resource->loadResource();
	});
}

GUID ResourceManager::findFileGuid(const GUID &resourceGuid)
{
	ResourceGuidList::iterator iter = resourceGuids.find(resourceGuid);
	if (iter != resourceGuids.end())
		return iter->second;
	return GUID::none;
}

TS_END_PACKAGE1()

