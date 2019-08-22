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

	return true;
}

void ViewerStateManager::deinitialize()
{

}

void ViewerStateManager::nextImage()
{
	{
		std::unique_lock<std::mutex> lock(mutex);

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
		std::unique_lock<std::mutex> lock(mutex);

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
	{
		std::unique_lock<std::mutex> lock(mutex);
		index = math::clamp(index, 0U, (SizeType)currentFileList.size());

		currentImageIndex = index;
		currentFilePath = currentFileList[currentImageIndex];
	}

	currentImageChangedSignal(currentImageIndex);
}

SizeType ViewerStateManager::getNumImages() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return (SizeType)currentFileList.size();
}

void ViewerStateManager::setSorting(SortingStyle sorting)
{
	std::unique_lock<std::mutex> lock(mutex);
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

std::vector<std::wstring> ViewerStateManager::getCurrentSortedFileList() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return currentFileList;
}

std::map<SizeType, std::wstring> ViewerStateManager::getFileListSlice(SizeType startIndex, SizeType endIndex)
{
	std::unique_lock<std::mutex> lock(mutex);
	return getFileListSliceUnsafe(startIndex, endIndex);
}

std::map<SizeType, std::wstring> ViewerStateManager::getFileListSliceUnsafe(SizeType startIndex, SizeType endIndex)
{
	std::map<SizeType, std::wstring> slice;
	if (!(startIndex < currentFileList.size() && endIndex < currentFileList.size()))
	{
		TS_ASSERT(!"StartIndex and/or EndIndex is out of bounds.");
		return slice;
	}

	const PosType fileListSize = currentFileList.size();
	SizeType numEntries = (startIndex <= endIndex) ? endIndex - startIndex : (SizeType)fileListSize - startIndex + endIndex;

	for (SizeType base = 0; base < numEntries; ++base)
	{
		SizeType index = (base + startIndex) % fileListSize;
		slice[index] = currentFileList[index];
	}

// 	if (startIndex < endIndex)
// 	{
// 		slice.insert(slice.end(), currentFileList.begin() + startIndex, currentFileList.begin() + endIndex);
// 	}
// 	else
// 	{
// 		slice.insert(slice.end(), currentFileList.begin() + startIndex, currentFileList.end());
// 		slice.insert(slice.end(), currentFileList.begin(), currentFileList.begin() + endIndex);
// 	}

	return slice;
}

std::map<SizeType, std::wstring> ViewerStateManager::getFileListSliceByOffsets(PosType startOffset, SizeType endOffset)
{
	TS_ASSERT(startOffset <= endOffset && "startOffset should be less than endOffset");
	TS_ASSERT(startOffset <= 0 && "startOffset should be less than or equal to zero.");
	TS_ASSERT(endOffset >= 1 && "endOffset should be greater than or equal to one.");

	std::unique_lock<std::mutex> lock(mutex);

	const SizeType fileListSize = (SizeType)currentFileList.size();

	PosType startIndex = currentImageIndex + startOffset;
	if (startIndex < 0)
		startIndex += fileListSize;

	SizeType endIndex = (currentImageIndex + endOffset) % fileListSize;

	return getFileListSliceUnsafe((SizeType)startIndex, (SizeType)endIndex);
}

SizeType ViewerStateManager::getCurrentImageIndex() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return currentImageIndex;
}

const std::wstring &ViewerStateManager::getCurrentFilepath() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return currentFilePath;
}

void ViewerStateManager::updateFileList()
{
	viewer::BackgroundFileScanner &BackgroundFileScanner = getGigaton<viewer::BackgroundFileScanner>();

	std::unique_lock<std::mutex> lock(mutex);
	currentFileList = BackgroundFileScanner.getFileList();
		
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

PosType ViewerStateManager::findFileIndexByName(const std::wstring &filepath, const std::vector<std::wstring> &filelist)
{
	PosType imageIndex = -1;

	std::vector<std::wstring>::const_iterator it = std::find(filelist.begin(), filelist.end(), filepath);
	if (it != filelist.end())
		imageIndex = std::distance(filelist.begin(), it);

	return imageIndex;
}

TS_END_PACKAGE2()



