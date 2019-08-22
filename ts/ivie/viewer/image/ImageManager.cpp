#include "Precompiled.h"
#include "ImageManager.h"

#include "ts/tessa/util/ContainerUtil.h"
#include "ts/tessa/math/Hash.h"
#include "ts/tessa/lang/StringUtils.h"

#include "ts/ivie/viewer/BackgroundFileScanner.h"
#include "ts/ivie/viewer/ViewerStateManager.h"

#include "ts/ivie/viewer/image/Image.h"

TS_DEFINE_MANAGER_TYPE(app::viewer::ImageManager);

TS_PACKAGE2(app, viewer)

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
	ViewerStateManager &vsm = getGigaton<ViewerStateManager>();
	
	currentImageChangedBind.connect(vsm.currentImageChangedSignal, &ThisClass::currentImageChanged, this);

	return true;
}

void ImageManager::deinitialize()
{
	for (ImageStorageList::iterator it = imageStorage.begin(); it != imageStorage.end(); ++it)
	{
		UniquePointer<Image> &image = it->second;
		if (image && !image->isUnloaded())
			image->unload();
	}
	imageStorage.clear();
}

std::wstring ImageManager::getStats()
{
	std::vector<std::wstring> stats;
	
	for (ImageStorageList::iterator it = imageStorage.begin(); it != imageStorage.end(); ++it)
	{
		UniquePointer<Image> &image = it->second;
		stats.push_back(image->getStats());
	}

	std::sort(stats.begin(), stats.end());

	return lang::utils::joinString(stats, L"\n");
}

Image *ImageManager::getCurrentImage()
{
	std::unique_lock<std::mutex> lock(mutex);

	ImageStorageList::const_iterator it = imageStorage.find(currentImageHash);
	if (it != imageStorage.end())
		return it->second.get();

	return nullptr;
}

void ImageManager::currentImageChanged(SizeType imageIndex)
{
	std::unique_lock<std::mutex> lock(mutex);

	currentImageIndex = imageIndex;

	const PosType numForwardBuffered = 2;
	const PosType numBackwardBuffered = 1;

	ViewerStateManager &vsm = getGigaton<ViewerStateManager>();

	std::vector<Uint32> activeImages;

	std::map<SizeType, std::wstring> imagesToLoad = vsm.getFileListSliceByOffsets(-numBackwardBuffered, numForwardBuffered);
	for (auto &it : imagesToLoad)
	{
		const SizeType index = it.first;
		const std::wstring &filepath = it.second;

		Uint32 imageHash = math::simpleHash32(filepath);
		activeImages.push_back(imageHash);

		bool isCurrentImage = (index == currentImageIndex);
		if (isCurrentImage)
			currentImageHash = imageHash;

		if (imageStorage[imageHash] == nullptr)
			imageStorage[imageHash] = makeUnique<Image>(filepath);

		UniquePointer<Image> &image = imageStorage[imageHash];

		if (image->isUnloaded())
		{
			TS_WPRINTF("--- Starting loading %s\n", filepath);
			image->startLoading(!isCurrentImage);
		}
		else if (image->isSuspended())
		{
			TS_WPRINTF("--- Resuming loading %s\n", filepath);
			image->resumeLoading();
		}
	}

	for (ImageStorageList::iterator it = imageStorage.begin(); it != imageStorage.end(); ++it)
	{
		Uint32 imageHash = it->first;
		UniquePointer<Image> &image = it->second;

		if (image->getState() == Image::Unloaded)
			continue;

		if (!ts::util::findIfContains(activeImages, imageHash))
		{
			TS_WPRINTF("--- Unloading %s\n", image->getFilepath());
			image->unload();
		}
		else if (imageHash != currentImageHash && image->getCurrentFrameIndex() > 0)
		{
			TS_WPRINTF("--- Restarting playback %s\n", image->getFilepath());
			image->restart(true);
		}
	}
}

TS_END_PACKAGE2()


