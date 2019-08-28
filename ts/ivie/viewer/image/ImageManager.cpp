#include "Precompiled.h"
#include "ImageManager.h"

#include "ts/tessa/util/ContainerUtil.h"
#include "ts/tessa/math/Hash.h"
#include "ts/tessa/lang/StringUtils.h"

#include "ts/ivie/viewer/BackgroundFileScanner.h"
#include "ts/ivie/viewer/ViewerStateManager.h"

#include "ts/tessa/resource/ResourceManager.h"
#include "ts/tessa/resource/ShaderResource.h"

#include "ts/tessa/thread/AbstractThreadEntry.h"
#include "ts/tessa/thread/Thread.h"
#include "ts/tessa/thread/ThreadScheduler.h"

#include "ts/ivie/viewer/image/Image.h"

#include "ts/tessa/profiling/SimpleScopedZoneTimer.h"

TS_DEFINE_MANAGER_TYPE(app::viewer::ImageManager);

TS_PACKAGE2(app, viewer)

class ImageManager::BackgroundImageUnloader : public thread::AbstractThreadEntry
{
	ImageManager *imageManager = nullptr;

	std::atomic_bool running = true;
	Thread *thread = nullptr;

	Mutex mutex;
	ConditionVariable condition;

	std::map<Uint32, Time> unloadQueue;

public:
	BackgroundImageUnloader(ImageManager *imageManager)
		: imageManager(imageManager)
	{
		running = true;
		condition.notifyAll();

		thread = Thread::createThread(this, "ImageManager::BackgroundImageUnloader");
	}

	~BackgroundImageUnloader()
	{
		running = false;
		if (thread != nullptr)
			Thread::joinThread(thread);
	}

	void addToQueue(Uint32 imageHash, TimeSpan delay)
	{
		TS_ASSERT(imageManager->imageStorage.find(imageHash) != imageManager->imageStorage.end() &&
			"Image hash not found in storage, don't try to unload images that aren't even loaded.");

		MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
		unloadQueue[imageHash] = Time::now() + delay;
	}

	void removeFromQueue(Uint32 imageHash)
	{
// 		TS_ASSERT(unloadQueue.find(imageHash) != unloadQueue.end() &&
// 			"Image hash not found in unload queue, don't try to cancel unloads.");

		MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
		unloadQueue.erase(imageHash);
	}

	void entry()
	{
		while (running)
		{
			MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
			condition.waitFor(lock, 200_ms, [this]()
			{
				return !running;// || !unloadQueue.empty();
			});
			if (!running)
				return;

			std::vector<SharedPointer<Image>> unloadables;

			for (auto it = unloadQueue.begin(); it != unloadQueue.end();)
			{
				if (Time::now() >= it->second)
				{
					unloadables.push_back(imageManager->imageStorage[it->first]);
					it = unloadQueue.erase(it);
				}
				else
				{
					++it;
				}
			}

			lock.unlock();

			for (SharedPointer<Image> &image : unloadables)
			{
				if (image != nullptr && !image->isUnloaded())
				{
					TS_WPRINTF("---- Unloading image %s\n", image->getFilepath());
					image->unload();
				}
			}
		}
	}

};

ImageManager::ImageManager()
{
	gigaton.registerClass(this);
}

ImageManager::~ImageManager()
{
	gigaton.unregisterClass(this);
}

bool ImageManager::initialize()
{
	prepareShaders();

	ViewerStateManager &vsm = getGigaton<ViewerStateManager>();
	currentImageChangedBind.connect(vsm.currentImageChangedSignal, &ThisClass::currentImageChanged, this);

	backgroundUnloader.reset(new BackgroundImageUnloader(this));

	return true;
}

void ImageManager::deinitialize()
{
	backgroundUnloader.reset();

	for (ImageStorageList::iterator it = imageStorage.begin(); it != imageStorage.end(); ++it)
	{
		SharedPointer<Image> &image = it->second;
		if (image && !image->isUnloaded())
			image->unload();
	}
	imageStorage.clear();
}

void ImageManager::update(const TimeSpan deltaTime)
{
// 	if (pendingImageUpdate && imageChangedTimer.getElapsedTime() >= 30_ms)
// 	{
// 		updateCurrentImage();
// 		pendingImageUpdate = false;
// 	}
}

std::wstring ImageManager::getStats()
{
	std::vector<std::wstring> stats;
	
	for (ImageStorageList::iterator it = imageStorage.begin(); it != imageStorage.end(); ++it)
	{
		SharedPointer<Image> &image = it->second;
		stats.push_back(image->getStats());
	}

	std::sort(stats.begin(), stats.end());

	return lang::utils::joinString(stats, L"\n");
}

Image *ImageManager::getCurrentImage() const
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());

	ImageStorageList::const_iterator it = imageStorage.find(currentImageHash);
	if (it != imageStorage.end())
		return it->second.get();

	return nullptr;
}

void ImageManager::prepareShaders()
{
	{
		const SizeType checkerPatternSize = 8;

		sf::RenderTexture rt;
		rt.create(checkerPatternSize * 2, checkerPatternSize * 2);
		rt.clear(sf::Color::White);

		const math::VC2 size = math::VC2((float)checkerPatternSize, (float)checkerPatternSize);
		sf::RectangleShape shape(size);
		shape.setPosition(0.f, 0.f);
		shape.setFillColor(sf::Color(190, 190, 190));
		rt.draw(shape);
		shape.setPosition(size);
		rt.draw(shape);
		rt.display();

		alphaCheckerPatternTexture = makeShared<sf::Texture>(rt.getTexture());
		if (alphaCheckerPatternTexture)
		{
			alphaCheckerPatternTexture->setRepeated(true);
			alphaCheckerPatternTexture->setSmooth(true);
		}
	}

	displayShaderFiles.insert(std::make_pair(DisplayShader_FreeImage, "shader/convert.frag"));
}

SharedPointer<sf::Shader> ImageManager::loadDisplayShader(DisplayShaderTypes type)
{
	SharedPointer<sf::Shader> displayShader = makeShared<sf::Shader>();
	TS_ASSERT(displayShader);
	if (displayShader == nullptr)
		return nullptr;

	TS_ASSERT(displayShaderFiles.find(type) != displayShaderFiles.end() && "Attempting to load an undefined display shader.");
	
	std::string filepath = resource::ResourceManager::getAbsoluteResourcePath(displayShaderFiles[type]);
	if (!displayShader->loadFromFile(filepath, sf::Shader::Fragment))
		return nullptr;

	TS_ASSERT(alphaCheckerPatternTexture);
	displayShader->setUniform("u_checkerPatternTexture", *alphaCheckerPatternTexture);
	
	return displayShader;
}

void ImageManager::currentImageChanged(SizeType imageIndex)
{
// 	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());

	currentImageIndex = imageIndex;
// 	pendingImageUpdate = true;
// 	imageChangedTimer.restart();

	updateCurrentImage();
}

void ImageManager::updateCurrentImage()
{
	const PosType numForwardBuffered = 2;
	const PosType numBackwardBuffered = 2;

	ViewerStateManager &vsm = getGigaton<ViewerStateManager>();

	std::vector<Uint32> activeImages;
	std::vector<ImageEntry> imagesToLoad = vsm.getListSliceForBuffering(numForwardBuffered, numBackwardBuffered);

// 	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	for (const ImageEntry &entry : imagesToLoad)
	{
		Uint32 imageHash = math::simpleHash32(entry.filepath);
		activeImages.push_back(imageHash);

		SharedPointer<Image> &image = imageStorage[imageHash];

		bool isCurrentImage = (entry.index == currentImageIndex);
		if (isCurrentImage)
			currentImageHash = imageHash;

		if (image == nullptr)
			image = makeShared<Image>(entry.filepath);

		if (image->isUnloaded())
		{
// 			TS_WPRINTF("--- Starting loading %s\n", entry.filepath);
			image->startLoading(!isCurrentImage);
		}
		else if (image->isSuspended() && isCurrentImage)
		{
// 			TS_WPRINTF("--- Resuming loading %s\n", entry.filepath);
			image->resumeLoading();
		}

		if (image->hasError())
			continue;

		image->setActive(isCurrentImage);
	}

	for (ImageStorageList::iterator it = imageStorage.begin(); it != imageStorage.end(); ++it)
	{
		Uint32 imageHash = it->first;
		SharedPointer<Image> &image = it->second;

		if (image->getState() == Image::Unloaded)
			continue;

		if (!ts::util::findIfContains(activeImages, imageHash))
		{
// 			TS_WPRINTF("--- Adding to unload queue %s\n", image->getFilepath());
// 			image->unload();
			image->suspendLoader();
			backgroundUnloader->addToQueue(imageHash, TimeSpan::fromMilliseconds(2000));
		}
		else if (imageHash != currentImageHash)
		{
// 			TS_WPRINTF("--- Restarting playback %s\n", image->getFilepath());
			image->restart(true);
			backgroundUnloader->removeFromQueue(imageHash);
		}
	}
}

TS_END_PACKAGE2()
