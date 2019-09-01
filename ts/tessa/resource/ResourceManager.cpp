#include "Precompiled.h"
#include "ts/tessa/resource/ResourceManager.h"

#include "ts/tessa/system/BaseApplication.h"
#include "ts/tessa/thread/ThreadScheduler.h"

#include "ts/tessa/resource/ImageResource.h"
#include "ts/tessa/resource/TextureResource.h"
#include "ts/tessa/resource/FontResource.h"
#include "ts/tessa/resource/SoundResource.h"
#include "ts/tessa/resource/MusicResource.h"
#include "ts/tessa/resource/ShaderResource.h"

#include "ts/tessa/file/FileUtils.h"

TS_DEFINE_MANAGER_TYPE(resource::ResourceManager);

TS_PACKAGE1(resource)

String ResourceManager::resourceRootDirectory = "";

std::atomic_bool ResourceManager::stop_flag;

ResourceManager::ResourceManager()
{
	gigaton.registerClass(this);
}

ResourceManager::~ResourceManager()
{
	gigaton.unregisterClass(this);
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

void ResourceManager::update(const TimeSpan deltaTime)
{

}

ImageResource *ResourceManager::loadImage(const String &uniqueResourceHandle, const String &filepath, const bool immediate)
{
	return loadResource<ImageResource>(uniqueResourceHandle, filepath, immediate);
}

TextureResource *ResourceManager::loadTexture(const String &uniqueResourceHandle, const String &filepath, const bool immediate)
{
	return loadResource<TextureResource>(uniqueResourceHandle, filepath, immediate);
}

FontResource *ResourceManager::loadFont(const String &uniqueResourceHandle, const String &filepath, const bool immediate)
{
	return loadResource<FontResource>(uniqueResourceHandle, filepath, immediate);
}

ShaderResource *ResourceManager::loadShader(const String &uniqueResourceHandle, const String &filepath, const bool immediate)
{
	return loadResource<ShaderResource>(uniqueResourceHandle, filepath, immediate);
}

MusicResource *ResourceManager::loadMusic(const String &uniqueResourceHandle, const String &filepath, const bool immediate)
{
	return loadResource<MusicResource>(uniqueResourceHandle, filepath, immediate);
}

SoundResource *ResourceManager::loadSound(const String &uniqueResourceHandle, const String &filepath, const bool immediate)
{
	return loadResource<SoundResource>(uniqueResourceHandle, filepath, immediate);
}

ImageResource *ResourceManager::getImage(const String &uniqueResourceHandle) const
{
	return getResource<ImageResource>(uniqueResourceHandle);
}

TextureResource *ResourceManager::getTexture(const String &uniqueResourceHandle) const
{
	return getResource<TextureResource>(uniqueResourceHandle);
}

FontResource *ResourceManager::getFont(const String &uniqueResourceHandle) const
{
	return getResource<FontResource>(uniqueResourceHandle);
}

ShaderResource *ResourceManager::getShader(const String &uniqueResourceHandle) const
{
	return getResource<ShaderResource>(uniqueResourceHandle);
}

MusicResource *ResourceManager::getMusic(const String &uniqueResourceHandle) const
{
	return getResource<MusicResource>(uniqueResourceHandle);
}

SoundResource *ResourceManager::getSound(const String &uniqueResourceHandle) const
{
	return getResource<SoundResource>(uniqueResourceHandle);
}

void ResourceManager::unloadAll()
{
	resources.clear();
}

bool ResourceManager::unloadResource(const String &uniqueResourceHandle)
{
	return unloadResourceByGuid(GUID(uniqueResourceHandle));
}

bool ResourceManager::unloadResourceByGuid(const GUID &resourceGuid)
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

bool ResourceManager::unloadResourceByFilePath(const String &filepath)
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

void ResourceManager::setResourceRootDirectory(const String &rootDirectory)
{
	ResourceManager::resourceRootDirectory = rootDirectory;
}

const String &ResourceManager::getResourceRootDirectory()
{
	return ResourceManager::resourceRootDirectory;
}

String ResourceManager::getAbsoluteResourcePath(const String &filepath)
{
	if (!file::isAbsolutePath(filepath))
		return file::joinPaths(ResourceManager::resourceRootDirectory, filepath);

	return filepath;
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
	TS_VERIFY_POINTERS(resource);

// 	if (ResourceManager::stop_flag.load(std::memory_order_relaxed))
// 		return;

	thread::ThreadScheduler &tm = getGigaton<thread::ThreadScheduler>();
	tm.scheduleOnce(thread::Priority_Normal, TimeSpan::zero, &ResourceManager::loadResourceTask, resource);
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

