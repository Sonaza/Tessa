#include "Precompiled.h"
#include "ViewerStateManager.h"

#include "ts/ivie/util/NaturalSort.h"
#include "ts/ivie/viewer/BackgroundFileScanner.h"

TS_DEFINE_MANAGER_TYPE(app::viewer::ViewerStateManager);

TS_PACKAGE2(app, viewer)

ViewerStateManager::ViewerStateManager()
{
	gigaton.registerClass(this);
}

ViewerStateManager::~ViewerStateManager()
{
	gigaton.unregisterClass(this);

}
bool ViewerStateManager::initialize()
{
	BackgroundFileScanner &fs = getGigaton<BackgroundFileScanner>();
	filelistChangedBind.connect(fs.filelistChangedSignal, &ThisClass::updateFileList, this);

	updateFileList();

	return true;
}

void ViewerStateManager::deinitialize()
{

}

void ViewerStateManager::nextImage()
{
	{
		MutexGuard lock(mutex);

		if (currentImageIndex + 1 >= (SizeType)currentFileList.size())
			currentImageIndex = 0;
		else
			currentImageIndex++;
		currentFilePath = currentFileList[currentImageIndex];
	}

	currentImageChangedSignal(currentImageIndex);
}

void ViewerStateManager::previousImage()
{
	{
		MutexGuard lock(mutex);

		if (currentImageIndex == 0)
			currentImageIndex = (SizeType)math::max(0, (int32_t)currentFileList.size() - 1);
		else
			currentImageIndex--;

		currentFilePath = currentFileList[currentImageIndex];
	}

	currentImageChangedSignal(currentImageIndex);
}

void ViewerStateManager::jumpToImage(SizeType index)
{
	if (currentFileList.empty())
		return;

	{
		MutexGuard lock(mutex);
		index = math::clamp(index, 0U, (SizeType)currentFileList.size());

		currentImageIndex = index;
		currentFilePath = currentFileList[currentImageIndex];
	}

	currentImageChangedSignal(currentImageIndex);
}

void ViewerStateManager::jumpToImageByFilename(const String &filename)
{
	MutexGuard lock(mutex);
	std::vector<String>::iterator it = std::find(currentFileList.begin(), currentFileList.end(), filename);
	if (it != currentFileList.end())
	{
		currentImageIndex = (SizeType)std::distance(currentFileList.begin(), it);
		currentFilePath = currentFileList[currentImageIndex];

		lock.unlock();

		currentImageChangedSignal(currentImageIndex);
	}
}

SizeType ViewerStateManager::getNumImages() const
{
	MutexGuard lock(mutex);
	return (SizeType)currentFileList.size();
}

void ViewerStateManager::setSorting(SortingStyle sorting)
{
	MutexGuard lock(mutex);
	if (currentSorting != sorting)
	{
		currentSorting = sorting;
		applySorting();

		PosType updatedIndex = findFileIndexByName(currentFilePath, currentFileList);
		if (updatedIndex != currentImageIndex)
		{
			if (updatedIndex >= 0)
			{
				currentImageIndex = (SizeType)updatedIndex;
			}
			else
			{
				currentImageIndex = currentImageIndex > 0 ? currentImageIndex - 1 : 0;
			}

			lock.unlock();
			currentImageChangedSignal(currentImageIndex);
		}
	}
}

const std::vector<String> &ViewerStateManager::getCurrentSortedFileList() const
{
	MutexGuard lock(mutex);
	return currentFileList;
}

const std::vector<ImageEntry> ViewerStateManager::getListSliceForBuffering(SizeType numForward, SizeType numBackward)
{
	std::vector<ImageEntry> result;

	MutexGuard lock(mutex);
	const SizeType fileListSize = (SizeType)currentFileList.size();
	if (fileListSize == 0)
		return result;

	// Always returns the current image
	SizeType numEntries = math::min(fileListSize, 1 + numForward + numBackward);
	result.reserve(numEntries);

	for (SizeType base = 0; base < numForward + 1; ++base)
	{
		SizeType index = (currentImageIndex + base) % fileListSize;
		result.push_back(ImageEntry{ currentFileList[index], index, ImageEntry::Buffering_Forwards });

		numEntries--;
		if (numEntries == 0)
			break;
	}

	if (numEntries > 0)
	{
		for (SizeType base = 0; base < numBackward; ++base)
		{
			SizeType index = (currentImageIndex + (fileListSize - 1 - (PosType)base)) % fileListSize;
			result.push_back(ImageEntry{ currentFileList[index], index, ImageEntry::Buffering_Backwards });
		
			numEntries--;
			if (numEntries == 0)
				break;
		}
	}

	return result;
}

SizeType ViewerStateManager::getCurrentImageIndex() const
{
	MutexGuard lock(mutex);
	return currentImageIndex;
}

const String &ViewerStateManager::getCurrentFilepath() const
{
	MutexGuard lock(mutex);
	return currentFilePath;
}

void ViewerStateManager::updateFileList()
{
	viewer::BackgroundFileScanner &backgroundFileScanner = getGigaton<viewer::BackgroundFileScanner>();

	MutexGuard lock(mutex);
	currentFileList = backgroundFileScanner.getFileList();
		
	applySorting();

	PosType updatedIndex = findFileIndexByName(currentFilePath, currentFileList);
	if (updatedIndex != currentImageIndex)
	{
		if (updatedIndex >= 0)
		{
			currentImageIndex = (SizeType)updatedIndex;
		}
		else
		{
			currentImageIndex = currentImageIndex > 0 ? currentImageIndex - 1 : 0;
		}

		lock.unlock();
		currentImageChangedSignal(currentImageIndex);
	}
}

void ViewerStateManager::applySorting()
{
	switch (currentSorting)
	{
		case SortByName:
			std::sort(currentFileList.begin(), currentFileList.end(), util::NaturalSort);
		break;

		case SortByExtension:
			std::sort(currentFileList.begin(), currentFileList.end(), util::NaturalSortByExtension);
		break;
	}
}

PosType ViewerStateManager::findFileIndexByName(const String &filepath, const std::vector<String> &filelist)
{
	PosType imageIndex = -1;

	std::vector<String>::const_iterator it = std::find(filelist.begin(), filelist.end(), filepath);
	if (it != filelist.end())
		imageIndex = std::distance(filelist.begin(), it);

	return imageIndex;
}

TS_END_PACKAGE2()



