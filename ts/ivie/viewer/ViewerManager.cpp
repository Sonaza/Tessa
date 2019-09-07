#include "Precompiled.h"
#include "ViewerManager.h"

#include "ts/tessa/util/ContainerUtil.h"
#include "ts/tessa/math/Hash.h"
#include "ts/tessa/string/StringUtils.h"
#include "ts/tessa/thread/AbstractThreadEntry.h"
#include "ts/tessa/thread/Thread.h"

#include "ts/tessa/resource/ResourceManager.h"

#include "ts/tessa/file/FileUtils.h"

#include "ts/ivie/viewer/SupportedFormats.h"
#include "ts/ivie/viewer/image/Image.h"
#include "ts/ivie/util/NaturalSort.h"

#include "ts/tessa/profiling/ZoneProfiler.h"

TS_DEFINE_MANAGER_TYPE(app::viewer::ViewerManager);

TS_PACKAGE2(app, viewer)

class ViewerManager::BackgroundImageUnloader : public thread::AbstractThreadEntry
{
	ViewerManager *viewerManager = nullptr;

	std::atomic_bool running = true;
	Thread *thread = nullptr;

	Mutex mutex;
	ConditionVariable condition;

	std::map<uint32, Time> unloadQueue;

public:
	BackgroundImageUnloader(ViewerManager *viewerManager)
		: viewerManager(viewerManager)
	{
		running = true;
		condition.notifyAll();

		thread = Thread::createThread(this, "ViewerManager::BackgroundImageUnloader");
	}

	~BackgroundImageUnloader()
	{
		running = false;
		if (thread != nullptr)
			Thread::joinThread(thread);
	}

	void addToQueue(uint32 imageHash, TimeSpan delay)
	{
		TS_ASSERT(viewerManager->imageStorage.find(imageHash) != viewerManager->imageStorage.end() &&
			"Image hash not found in storage, don't try to unload images that aren't even loaded.");

		MutexGuard lock(mutex);
		unloadQueue[imageHash] = Time::now() + delay;
	}

	void removeFromQueue(uint32 imageHash)
	{
// 		TS_ASSERT(unloadQueue.find(imageHash) != unloadQueue.end() &&
// 			"Image hash not found in unload queue, don't try to cancel unloads.");

		MutexGuard lock(mutex);
		unloadQueue.erase(imageHash);
	}

	void entry()
	{
		while (running)
		{
			MutexGuard lock(mutex);
			condition.waitFor(lock, 50_ms, [this]()
			{
				return !running;// || !unloadQueue.empty();
			});
			if (!running)
				return;

// 			TS_ZONE();

			std::vector<SharedPointer<Image>> unloadables;

			for (auto it = unloadQueue.begin(); it != unloadQueue.end();)
			{
				if (Time::now() >= it->second)
				{
					unloadables.push_back(viewerManager->imageStorage[it->first]);
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

			Thread::sleep(10_ms);
		}
	}

};

std::atomic_bool ViewerManager::quitting = false;

ViewerManager::ViewerManager()
{
	gigaton.registerClass(this);
}

ViewerManager::~ViewerManager()
{
	gigaton.unregisterClass(this);

}
bool ViewerManager::initialize()
{
	TS_ZONE();

	threadScheduler = &getGigaton<thread::ThreadScheduler>();
	
	prepareShaders();

	allowedExtensions = viewer::SupportedFormats::getSupportedFormatExtensions();

	backgroundUnloader.reset(new BackgroundImageUnloader(this));

	return true;
}

void ViewerManager::deinitialize()
{
	TS_ZONE();

	quitting = true;

	if (scannerTaskId != thread::InvalidTaskId)
		threadScheduler->cancelTask(scannerTaskId);

	backgroundUnloader.reset();

	for (ImageStorageList::iterator it = imageStorage.begin(); it != imageStorage.end(); ++it)
	{
		SharedPointer<Image> &image = it->second;
		if (image && !image->isUnloaded())
			image->unload();
	}
	imageStorage.clear();
}

void ViewerManager::update(const TimeSpan deltaTime)
{
	TS_ZONE();

	if (pendingImageUpdate)
	{
		{
			MutexGuard lock(mutex);

			if (pendingImageIndex != -1 && !currentFileList.empty())
			{
				current.imageIndex = (SizeType)pendingImageIndex;
				current.filepath = currentFileList[current.imageIndex];
			}

			updateCurrentImage();

			pendingImageUpdate = false;
		}

		currentImageChangedSignal(currentImage);
	}
}

void ViewerManager::setFilepath(const String &filepath)
{
	TS_ZONE();

	const String directoryPath = file::getDirname(filepath);
	TS_ASSERT(!directoryPath.isEmpty());

	if (currentDirectoryPath == directoryPath)
	{
		if (file::isFile(filepath))
			jumpToImageByFilename(filepath);
		return;
	}

	if (scannerTaskId != thread::InvalidTaskId)
		threadScheduler->cancelTask(scannerTaskId);

	currentDirectoryPath = directoryPath;

	updateFilelist(currentDirectoryPath, false);

	if (file::isFile(filepath))
		jumpToImageByFilename(filepath);
	else
		jumpToImage(0);

	pendingImageUpdate = true;

	scannerTaskId = threadScheduler->scheduleWithInterval(
		thread::Priority_Normal,
		TimeSpan::fromMilliseconds(2000),
		&ViewerManager::updateFilelist, this, directoryPath, true);
}

const String &ViewerManager::getFilepath() const
{
	return currentDirectoryPath;
}

void ViewerManager::setRecursiveScan(bool recursiveEnabled)
{
	scanStyle = !recursiveEnabled ? file::FileListStyle_Files : file::FileListStyle_Files_Recursive;
	updateFilelist(currentDirectoryPath, true);
}

//////////////////////////////////////////////////////

void ViewerManager::jumpToImage(SizeType index)
{
	TS_ZONE();

	if (currentFileList.empty())
		return;

	const SizeType numImagesTotal = (SizeType)currentFileList.size();
	if (index >= numImagesTotal)
		return;

	pendingImageIndex = index;
	pendingImageUpdate = true;
}

void ViewerManager::jumpToImageByFilename(const String &filename)
{
	PosType index = findFileIndexByName(filename, currentFileList);
	if (index >= 0)
		jumpToImage((SizeType)index);
	else
		jumpToImage(0);
}

void ViewerManager::nextImage()
{
	changeImage(1);
}

void ViewerManager::previousImage()
{
	changeImage(-1);
}

void ViewerManager::changeImage(int32 amount)
{
	if (amount == 0)
		return;

	const SizeType numImagesTotal = (SizeType)currentFileList.size();
	if (numImagesTotal == 0)
		return;

	SizeType index = (current.imageIndex + numImagesTotal + amount) % numImagesTotal;
	jumpToImage(index);
}

SizeType ViewerManager::getCurrentImageIndex() const
{
	MutexGuard lock(mutex);
	return current.imageIndex;
}

SizeType ViewerManager::getNumImages() const
{
	MutexGuard lock(mutex);
	return (SizeType)currentFileList.size();
}

const String &ViewerManager::getCurrentFilepath() const
{
	MutexGuard lock(mutex);
	return current.filepath;
}

//////////////////////////////////////////////////////

void ViewerManager::setSorting(SortingStyle sorting)
{
	MutexGuard lock(mutex);
	if (currentSorting != sorting)
	{
		currentSorting = sorting;

		applySorting(currentFileList);
		ensureImageIndex();
	}
}

//////////////////////////////////////////////////////

const std::vector<ImageEntry> ViewerManager::getListSliceForBuffering(SizeType numForward, SizeType numBackward)
{
	TS_ZONE();

	std::vector<ImageEntry> result;

	const SizeType fileListSize = (SizeType)currentFileList.size();
	if (fileListSize == 0)
		return result;

	// Always returns the current image
	SizeType numEntries = math::min(fileListSize, 1 + numForward + numBackward);
	result.reserve(numEntries);

	for (SizeType base = 0; base < numForward + 1; ++base)
	{
		SizeType index = (current.imageIndex + base) % fileListSize;
		result.push_back(ImageEntry{ currentFileList[index], index, ImageEntry::Buffering_Forwards });

		numEntries--;
		if (numEntries == 0)
			break;
	}

	if (numEntries > 0)
	{
		for (SizeType base = 0; base < numBackward; ++base)
		{
			SizeType index = (current.imageIndex + (fileListSize - 1 - (PosType)base)) % fileListSize;
			result.push_back(ImageEntry{ currentFileList[index], index, ImageEntry::Buffering_Backwards });
		
			numEntries--;
			if (numEntries == 0)
				break;
		}
	}

	return result;
}

bool ViewerManager::isExtensionAllowed(const String &filename)
{
	const String ext = file::getExtension(filename);
	return std::find(allowedExtensions.begin(), allowedExtensions.end(), ext) != allowedExtensions.end();
}

bool ViewerManager::updateFilelist(const String directoryPath, bool ensureIndex)
{
	if (quitting)
		return false;

	TS_ZONE();

	file::FileList lister(directoryPath, true, scanStyle);

	std::vector<file::FileEntry> files = lister.getFullListing();

	std::vector<String> templist;
	templist.reserve(files.size());

	for (file::FileEntry &file : files)
	{
		if (isExtensionAllowed(file.getFilepath()))
		{
			templist.push_back({
				file.getFullFilepath()
			});
		}
	}

	applySorting(templist);

	if ((templist.size() != currentFileList.size()) || (templist != currentFileList))
	{
		{
			TS_ZONE_NAMED("Copying filelist");
			MutexGuard lock(mutex);
			currentFileList = std::move(templist);
		}

		if (ensureIndex)
			ensureImageIndex();

		filelistChangedSignal((SizeType)currentFileList.size());
	}

	Thread::sleep(5000_ms);

	return true;
}

void ViewerManager::applySorting(std::vector<String> &filelist)
{
	TS_ZONE();

	switch (currentSorting)
	{
		case SortByName:
			std::sort(filelist.begin(), filelist.end(), util::naturalSort);
		break;

		case SortByExtension:
			std::sort(filelist.begin(), filelist.end(), util::naturalSortByExtension);
		break;
	}
}

void ViewerManager::ensureImageIndex()
{
	if (currentFileList.empty())
	{
		current = DisplayState();
		pendingImageUpdate = true;
		pendingImageIndex = -1;
		return;
	}

	PosType updatedIndex = findFileIndexByName(current.filepath, currentFileList);
	if (updatedIndex != current.imageIndex)
	{
		if (updatedIndex >= 0)
		{
			current.imageIndex = (SizeType)updatedIndex;
		}
		else
		{
			jumpToImage((SizeType)(current.imageIndex > 0 ? current.imageIndex - 1 : 0));
		}
	}
}

PosType ViewerManager::findFileIndexByName(const String &filepath, const std::vector<String> &filelist)
{
	PosType imageIndex = -1;

	std::vector<String>::const_iterator it = std::find(filelist.begin(), filelist.end(), filepath);
	if (it != filelist.end())
		imageIndex = std::distance(filelist.begin(), it);

	return imageIndex;
}


String ViewerManager::getStats()
{
	std::vector<String> stats;

	for (ImageStorageList::iterator it = imageStorage.begin(); it != imageStorage.end(); ++it)
	{
		SharedPointer<Image> &image = it->second;
		stats.push_back(image->getStats());
	}

	std::sort(stats.begin(), stats.end());

	return string::joinString(stats, "\n");
}

SharedPointer<Image> ViewerManager::getCurrentImage() const
{
	TS_ZONE();

	MutexGuard lock(mutex);
	return currentImage;
}

void ViewerManager::prepareShaders()
{
	{
		const SizeType checkerPatternSize = 8;
		const SizeType size = checkerPatternSize * 2;

		// Generate array space for a checker pattern, something like this
		//   XX..
		//   ..XX
		std::vector<uint8> patternTexture(size * size * 4,  255);
		for (SizeType y = 0; y < size; ++y)
		{
			for (SizeType x = 0; x < size; ++x)
			{
				if ((y < checkerPatternSize && x < checkerPatternSize) ||
					(y >= checkerPatternSize && x >= checkerPatternSize))
				{
					patternTexture[(y * size + x) * 4 + 0] = 190;
					patternTexture[(y * size + x) * 4 + 1] = 190;
					patternTexture[(y * size + x) * 4 + 2] = 190;
				}
			}
		}

		alphaCheckerPatternTexture.create(size, size);
		alphaCheckerPatternTexture.update(&patternTexture[0], size, size, 0, 0);
		alphaCheckerPatternTexture.setRepeated(true);
	}

	displayShaderFiles.insert(std::make_pair(DisplayShader_FreeImage, "shader/convert_freeimage.frag"));
	displayShaderFiles.insert(std::make_pair(DisplayShader_Webm, "shader/convert_webm.frag"));
}

SharedPointer<sf::Shader> ViewerManager::loadDisplayShader(DisplayShaderTypes type)
{
	SharedPointer<sf::Shader> displayShader = makeShared<sf::Shader>();
	TS_ASSERT(displayShader);
	if (displayShader == nullptr)
		return nullptr;

	TS_ASSERT(displayShaderFiles.find(type) != displayShaderFiles.end() && "Attempting to load an undefined display shader.");

	String filepath = resource::ResourceManager::getAbsoluteResourcePath(displayShaderFiles[type]);
	if (!displayShader->loadFromFile(filepath, sf::Shader::Fragment))
		return nullptr;

	if (type == DisplayShader_FreeImage)
	{
		displayShader->setUniform("u_checkerPatternTexture", alphaCheckerPatternTexture);
	}

	return displayShader;
}

void ViewerManager::updateCurrentImage()
{
	TS_ZONE();

	const PosType numForwardBuffered = 2;
	const PosType numBackwardBuffered = 2;

	std::vector<uint32> activeImages;
	std::vector<ImageEntry> imagesToLoad = getListSliceForBuffering(numForwardBuffered, numBackwardBuffered);

	if (imagesToLoad.empty())
		currentImage = nullptr;

	uint32 currentImageHash = 0;

	for (const ImageEntry &entry : imagesToLoad)
	{
		uint32 imageHash = math::simpleHash32(entry.filepath);
		activeImages.push_back(imageHash);

		SharedPointer<Image> &image = imageStorage[imageHash];
		if (image == nullptr)
			image = makeShared<Image>(entry.filepath);

		bool isCurrentImage = (entry.index == current.imageIndex);
		if (isCurrentImage)
		{
			currentImageHash = imageHash;
			currentImage = image;
		}

		if (image->hasError())
			continue;

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
		{
			TS_WPRINTF("Couldn't load image: %s\n", image->getErrorText());
			continue;
		}

		image->setActive(isCurrentImage);
	}

	for (ImageStorageList::iterator it = imageStorage.begin(); it != imageStorage.end(); ++it)
	{
		uint32 imageHash = it->first;
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



