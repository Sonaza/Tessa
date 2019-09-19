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
#include "ts/ivie/image/Image.h"
#include "ts/ivie/util/NaturalSort.h"

#include "ts/tessa/profiling/ZoneProfiler.h"

TS_DEFINE_MANAGER_TYPE(app::viewer::ViewerManager);

TS_PACKAGE2(app, viewer)

class ViewerManager::BackgroundImageUnloader : public thread::AbstractThreadEntry
{
	ViewerManager *viewerManager = nullptr;

	std::atomic_bool running = true;
	Thread *thread = nullptr;

	ConditionVariable condition;

	std::map<uint32, Time> unloadQueue;

public:
	Mutex mutex;

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

		unloadQueue[imageHash] = Time::now() + delay;
	}

	void removeFromQueue(uint32 imageHash)
	{
// 		TS_ASSERT(unloadQueue.find(imageHash) != unloadQueue.end() &&
// 			"Image hash not found in unload queue, don't try to cancel unloads.");

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

			std::vector<SharedPointer<image::Image>> unloadables;

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

			for (SharedPointer<image::Image> &image : unloadables)
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
		threadScheduler->cancelTask(scannerTaskId, true);

	backgroundUnloader.reset();

	for (ImageStorageList::iterator it = imageStorage.begin(); it != imageStorage.end(); ++it)
	{
		SharedPointer<image::Image> &image = it->second;
		if (image && !image->isUnloaded())
			image->unload();
	}
	imageStorage.clear();
}

void ViewerManager::update(const TimeSpan deltaTime)
{
	TS_ZONE();

	fileWatcher.update();

	if (pendingImageUpdate)
	{
		{
			MutexGuard lock(mutex);

			SizeType previousImageIndex = current.imageIndex;

			if (pendingImageIndex != -1 && !currentFileList.empty())
			{
				current.imageIndex = (SizeType)pendingImageIndex;
				current.filepath = currentFileList[current.imageIndex];
			}

			updateCurrentImage(previousImageIndex);

			pendingImageUpdate = false;
		}

		currentImageChangedSignal(currentImage);
	}
}

void ViewerManager::watchNotify(const std::vector<file::FileNotifyEvent> &notifyEvents)
{
	MutexGuard lock(mutex, thread::DeferLock);
	
	bool sortNeeded = false;
	bool ensureImageIndexNeeded = false;

	for (const file::FileNotifyEvent &notifyEvent : notifyEvents)
	{
		if (!isExtensionAllowed(notifyEvent.name))
			continue;

		if (!lock.isLocked())
			lock.lock();

		switch (notifyEvent.flag)
		{
			case file::FileNotify_FileAdded:
			{
				currentFileList.push_back(notifyEvent.name);
			}
			break;

			case file::FileNotify_FileRemoved:
			{
				if (current.filepath == notifyEvent.name)
					previousImage();

				auto it = std::find(currentFileList.begin(), currentFileList.end(), notifyEvent.name);
				if (it != currentFileList.end())
					currentFileList.erase(it);
			}
			break;

			case file::FileNotify_FileRenamed:
			{
				TS_WPRINTF("File renamed! %s -> %s\n", notifyEvent.name, notifyEvent.lastName);

				auto it = std::find(currentFileList.begin(), currentFileList.end(), notifyEvent.lastName);
				if (it != currentFileList.end())
				{
					*it = notifyEvent.name;
					sortNeeded = true;
				}

				if (current.filepath == notifyEvent.lastName)
				{
					current.filepath = notifyEvent.name;
					ensureImageIndexNeeded = true;
				}
			}
			break;

			default: /* boop */ break;
		}
	}

	if (sortNeeded)
		applySorting(currentFileList);

	if (ensureImageIndexNeeded)
		ensureImageIndex();
}

void ViewerManager::resetFileWatcher(bool recursive)
{
	if (fileWatcher.isWatching())
	{
		watchNotifyBind.disconnect();
		fileWatcher.reset();
	}

	fileWatcher.watch(currentDirectoryPath, recursive, file::FileWatch_FileChanges | file::FileWatch_DirectoryChanges);
	watchNotifyBind.connect(fileWatcher.notifySignal, &ThisClass::watchNotify, this);
}

void ViewerManager::setFilepath(const String &filepath)
{
	TS_ZONE();

	if (!file::exists(filepath))
	{
		TS_WLOG_ERROR("Given filepath does not exist. Path: %s.", filepath);
		return;
	}

	String directoryPath = file::getDirname(filepath);
	if (directoryPath.isEmpty())
		directoryPath = file::getWorkingDirectory();

	if (getIsRecursiveScan() && file::pathIsSubpath(currentDirectoryPath, directoryPath))
	{
		if (file::isFile(filepath))
		{
			jumpToImageByFilename(filepath);
		}
		else if (file::isDirectory(filepath))
		{
			jumpToImageByDirectory(filepath);
		}
		
		return;
	}

	if (scannerTaskId != thread::InvalidTaskId)
	{
		threadScheduler->cancelTask(scannerTaskId, true);
		scannerTaskId = thread::InvalidTaskId;
	}

	firstScanComplete = false;
	currentDirectoryPath = directoryPath;

	resetFileWatcher(getIsRecursiveScan());

	IndexingAction action = IndexingAction_KeepCurrentFile;

	if (file::isFile(filepath) && isExtensionAllowed(filepath))
	{
		const String relativePath = file::stripRootPath(filepath, currentDirectoryPath);

		currentFileList.clear();
		currentFileList.push_back(relativePath);
		pendingImageIndex = -1;
		current.imageIndex = 0;
		current.filepath = relativePath;
		pendingImageUpdate = true;
	}
	else
	{
		action = IndexingAction_Reset;
	}

	scannerTaskId = threadScheduler->scheduleOnce(
		thread::Priority_Critical,
		TimeSpan::zero,
		&ThisClass::updateFilelist, this, directoryPath, true, false, action
	).getTaskId();
}

const String &ViewerManager::getFilepath() const
{
	return currentDirectoryPath;
}

bool ViewerManager::getIsRecursiveScan() const
{
	return scanStyle == file::FileListStyle_Files_Recursive;
}

void ViewerManager::setRecursiveScan(bool recursiveEnabled, bool immediateRescan)
{
	scanStyle = !recursiveEnabled ? file::FileListStyle_Files : file::FileListStyle_Files_Recursive;

	if (!currentDirectoryPath.isEmpty() && immediateRescan)
	{
		if (scannerTaskId != thread::InvalidTaskId)
		{
			threadScheduler->cancelTask(scannerTaskId, true);
			scannerTaskId = thread::InvalidTaskId;
		}

		resetFileWatcher(getIsRecursiveScan());

		firstScanComplete = false;

		scannerTaskId = threadScheduler->scheduleOnce(
			thread::Priority_Critical,
			TimeSpan::zero,
			&ThisClass::updateFilelist, this, currentDirectoryPath, false, true, IndexingAction_KeepCurrentFile
		).getTaskId();
	}
}

//////////////////////////////////////////////////////

void ViewerManager::jumpToImage(SizeType index)
{
	TS_ZONE();

	if (currentFileList.empty())
		return;

	const SizeType numImagesTotal = (SizeType)currentFileList.size();
	if (index >= numImagesTotal)
		index = numImagesTotal - 1;

	if (index == current.imageIndex)
		return;

	pendingImageIndex = index;
	pendingImageUpdate = true;
}

void ViewerManager::jumpToImageByFilename(const String &filename)
{
	const String relativePath = file::stripRootPath(filename, currentDirectoryPath);

	PosType index = findFileIndexByName(relativePath, currentFileList);
	if (index >= 0)
		jumpToImage((SizeType)index);
	else
		jumpToImage(0);
}

void ViewerManager::jumpToImageByDirectory(const String &filename)
{
	const String relativePath = file::stripRootPath(filename, currentDirectoryPath);

	auto pred = [&](const String &path)
	{
		return path.find(relativePath) == 0;
	};

	PosType index = -1;

	auto it = std::find_if(currentFileList.begin(), currentFileList.end(), pred);
	if (it != currentFileList.end())
	{
		index = std::distance(currentFileList.begin(), it);
	}

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

bool ViewerManager::isScanningFiles() const
{
	return scanningFiles.load();
}

bool ViewerManager::isFirstScanComplete() const
{
	return firstScanComplete;
}

const String ViewerManager::getCurrentFilepath(bool absolute) const
{
	MutexGuard lock(mutex);
	return absolute ? file::joinPaths(currentDirectoryPath, current.filepath) : current.filepath;
}

//////////////////////////////////////////////////////

void ViewerManager::setSorting(SortingStyle sorting)
{
	MutexGuard lock(mutex);
	if (currentSortingStyle != sorting)
	{
		currentSortingStyle = sorting;

		if (!currentFileList.empty())
		{
			applySorting(currentFileList);
			ensureImageIndex();

			updateCurrentImage(current.imageIndex);

			filelistChangedSignal((SizeType)currentFileList.size());
		}
	}
}

SortingStyle ViewerManager::getSorting() const
{
	return currentSortingStyle;
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

bool ViewerManager::updateFilelist(const String directoryPath,
	bool directoryChanged, bool allowFullRecursive, IndexingAction indexingAction)
{
	TS_ZONE();

	TS_WPRINTF("updateFilelist\n");
	TS_WPRINTF("  directoryPath      : %s\n", directoryPath);
	TS_WPRINTF("  directoryChanged   : %d\n", (int)directoryChanged);
	TS_WPRINTF("  allowFullRecursive : %d\n", (int)allowFullRecursive);
	TS_WPRINTF("  indexingAction     : %d\n", (int)indexingAction);
	TS_WPRINTF("\n");

	const thread::SchedulerTaskId taskId = thread::ThreadScheduler::getCurrentThreadTaskId();

	if (quitting)
		return false;

	if (!file::exists(directoryPath) || !file::isDirectory(directoryPath))
	{
		TS_WLOG_ERROR("Directory path does not exist. Path: %s", directoryPath);

		{
			TS_ZONE_NAMED("Copying filelist");
			MutexGuard lock(mutex);
			currentFileList.clear();

			currentDirectoryPath.clear();
		}

		pendingImageIndex = 0;
		pendingImageUpdate = true;

		filelistChangedSignal(0U);
		
		return false;
	}

	scanningFiles = true;

	std::vector<String> templist;
	file::FileListStyle listScanStyle = allowFullRecursive ? scanStyle : file::FileListStyle_Files;

	while (!quitting)
	{
		file::FileList lister(directoryPath, true, listScanStyle);

		file::FileEntry entry;
		while (lister.next(entry))
		{
			if (quitting)
				return false;

			if (threadScheduler->isTaskCancelled(taskId))
			{
				TS_PRINTF("Task cancelled, skedaddlar!\n");
				return false;
			}

			if (isExtensionAllowed(entry.getFilepath()))
			{
				templist.push_back({
					entry.getFilepath()
				});
			}
		}

		if (templist.empty() && scanStyle == file::FileListStyle_Files_Recursive && listScanStyle != scanStyle)
		{
			listScanStyle = scanStyle;
			continue;
		}
		break;
	}

	if (quitting || threadScheduler->isTaskCancelled(taskId))
		return false;

	applySorting(templist);

	if (quitting)
		return false;

	if (directoryChanged || (templist.size() != currentFileList.size()) || (templist != currentFileList))
	{
		{
			TS_ZONE_NAMED("Copying filelist");
			MutexGuard lock(mutex);
			currentFileList = std::move(templist);
		}

		switch (indexingAction)
		{
			case IndexingAction_DoNothing:
				// See me doing nothing here
			break;

			case IndexingAction_KeepCurrentFile:
				ensureImageIndex();
			break;

			case IndexingAction_Reset:
				TS_PRINTF("ASDASDASD\n");
				pendingImageIndex = 0;
				pendingImageUpdate = true;
			break;
		}

		if (directoryChanged)
		{
			TS_PRINTF("DSGXCVXCV\n");
			pendingImageIndex = 0;
			pendingImageUpdate = true;
		}

		filelistChangedSignal((SizeType)currentFileList.size());
	}

	scanningFiles = false;

	if (quitting)
		return false;

	if (allowFullRecursive == false && scanStyle == file::FileListStyle_Files_Recursive)
	{
		scannerTaskId = threadScheduler->scheduleOnce(
			thread::Priority_Normal,
			TimeSpan::zero,
			&ViewerManager::updateFilelist, this, directoryPath, false, true, IndexingAction_KeepCurrentFile
		).getTaskId();
	}
	else if (allowFullRecursive == true || scanStyle != file::FileListStyle_Files_Recursive)
	{
		scannerTaskId = thread::InvalidTaskId;
		firstScanComplete = true;
	}

	return true;
}

void ViewerManager::applySorting(std::vector<String> &filelist)
{
	TS_ZONE();

	switch (currentSortingStyle)
	{
		case SortingStyle_ByName:
			std::sort(filelist.begin(), filelist.end(), util::naturalSort);
		break;

		case SortingStyle_ByExtension:
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
	TS_WPRINTF("File: %s   Updated index: %lld\n", current.filepath, updatedIndex);
	if (updatedIndex != current.imageIndex)
	{
		if (updatedIndex >= 0)
		{
			current.imageIndex = (SizeType)updatedIndex;
		}
		else
		{
			SizeType index = (SizeType)(current.imageIndex > 0 ? current.imageIndex - 1 : 0);
			index = math::min(index, (SizeType)currentFileList.size() - 1);
			jumpToImage(index);
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
		SharedPointer<image::Image> &image = it->second;
		stats.push_back(image->getStats());
	}

	std::sort(stats.begin(), stats.end());

	return string::joinString(stats, "\n");
}

SharedPointer<image::Image> ViewerManager::getCurrentImage() const
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
	displayShaderFiles.insert(std::make_pair(DisplayShader_Webm,      "shader/convert_webm.frag"));
}

SharedPointer<resource::ShaderResource> ViewerManager::loadDisplayShader(DisplayShaderTypes type)
{
	TS_ASSERT(displayShaderFiles.find(type) != displayShaderFiles.end() && "Attempting to load an undefined display shader.");

	SharedPointer<resource::ShaderResource> displayShader = makeShared<resource::ShaderResource>(displayShaderFiles[type]);
	TS_ASSERT(displayShader);
	if (displayShader == nullptr)
		return nullptr;

	if (!displayShader->loadResource())
		return nullptr;

	sf::Shader *shader = displayShader->getResource().get();

	if (type == DisplayShader_FreeImage)
	{
		shader->setUniform("u_checkerPatternTexture", alphaCheckerPatternTexture);
	}

	return displayShader;
}

void ViewerManager::updateCurrentImage(SizeType previousImageIndex)
{
	TS_ZONE();

	const PosType numForwardBuffered = 2;
	const PosType numBackwardBuffered = 2;

	std::vector<uint32> activeImages;
	std::vector<ImageEntry> imagesToLoad = getListSliceForBuffering(numForwardBuffered, numBackwardBuffered);

	if (imagesToLoad.empty())
		currentImage = nullptr;

	uint32 currentImageHash = 0;

	const uint32 currentPathHash = !currentDirectoryPath.isEmpty() ? math::simpleHash32(currentDirectoryPath) : 0;

	for (const ImageEntry &entry : imagesToLoad)
	{
		uint32 imageHash = math::hashCombine(currentPathHash, entry.filepath);

		activeImages.push_back(imageHash);

		SharedPointer<image::Image> &image = imageStorage[imageHash];
		if (image == nullptr)
		{
			String absolutePath = file::joinPaths(currentDirectoryPath, entry.filepath);
			image = makeShared<image::Image>(absolutePath);
		}

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
			image->startLoading(!isCurrentImage);
		}
		else if (isCurrentImage && image->isSuspended())
		{
			image->resumeLoading();
		}
		else if (entry.index == previousImageIndex && !isCurrentImage)
		{
			image->restart(true);
		}
		
		if (image->hasError())
		{
			TS_WPRINTF("Couldn't load image: %s\n", image->getErrorText());
			continue;
		}

		image->setActive(isCurrentImage);
	}

	std::vector<uint32> newlyActiveImages;
	for (uint32 imageHash : activeImages)
	{
		if (!ts::util::findIfContains(lastActiveImages, imageHash))
			newlyActiveImages.push_back(imageHash);
	}

	std::vector<uint32> newlyInactiveImages;
	for (uint32 imageHash : lastActiveImages)
	{
		if (!ts::util::findIfContains(activeImages, imageHash))
			newlyInactiveImages.push_back(imageHash);
	}

	lastActiveImages = activeImages;

	{
		MutexGuard lock(backgroundUnloader->mutex);

		for (const uint32 imageHash : newlyActiveImages)
		{
	 		SharedPointer<image::Image> &image = imageStorage[imageHash];
			
			if (image->getState() == image::Image::Unloaded)
				continue;

			image->restart(true);
			backgroundUnloader->removeFromQueue(imageHash);
		}

		for (const uint32 imageHash : newlyInactiveImages)
		{
			SharedPointer<image::Image> &image = imageStorage[imageHash];

			if (image->getState() == image::Image::Unloaded)
				continue;

			image->suspendLoader();
			backgroundUnloader->addToQueue(imageHash, 2000_ms);
		}
	}
}

TS_END_PACKAGE2()



