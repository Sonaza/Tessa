#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

#include "ts/tessa/lang/Signal.h"

TS_PACKAGE2(app, viewer)

class ViewerStateManager : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(app::viewer::ViewerStateManager);

public:
	ViewerStateManager();
	~ViewerStateManager();

	virtual bool initialize();
	virtual void deinitialize();

	SizeType getCurrentImageIndex() const;
	const std::wstring &getCurrentFilepath() const;

	void nextImage();
	void previousImage();

	void jumpToImage(SizeType index);
	SizeType getNumImages() const;

	enum SortingStyle
	{
		SortByName,
		SortByExtension,
	};
	void setSorting(SortingStyle sorting);

	std::vector<std::wstring> getCurrentSortedFileList() const;

	// If startIndex > endIndex, it'll wrap around
	std::map<SizeType, std::wstring> getFileListSlice(SizeType startIndex, SizeType endIndex);
	std::map<SizeType, std::wstring> getFileListSliceByOffsets(PosType startOffset, SizeType endOffset);

	lang::Signal<SizeType> currentImageChangedSignal;

private:
	std::map<SizeType, std::wstring> getFileListSliceUnsafe(SizeType startIndex, SizeType endIndex);

	void updateFileList();

	void applySorting();
	PosType findFileIndexByName(const std::wstring &filepath, const std::vector<std::wstring> &filelist);

	SortingStyle currentSorting = SortByName;

	lang::SignalBind filelistChangedBind;

	SizeType currentImageIndex;
	std::wstring currentFilePath;

	mutable std::mutex mutex;
	std::vector<std::wstring> currentFileList;
};

TS_END_PACKAGE2()
