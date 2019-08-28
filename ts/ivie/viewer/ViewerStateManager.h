#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

TS_PACKAGE2(app, viewer)

struct ImageEntry 
{
	std::wstring filepath;
	SizeType index;

	enum Buffering
	{
		Buffering_Forwards,
		Buffering_Backwards,
	};
	Buffering buffering;
};

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
	void jumpToImageByFilename(const std::wstring &filename);
	SizeType getNumImages() const;

	enum SortingStyle
	{
		SortByName,
		SortByExtension,
	};
	void setSorting(SortingStyle sorting);

	const std::vector<std::wstring> &getCurrentSortedFileList() const;

	// Retrieves a list of entries for buffering. 
	const std::vector<ImageEntry> getListSliceForBuffering(SizeType numForward, SizeType numBackward);

	lang::Signal<SizeType> currentImageChangedSignal;

private:
	void updateFileList();

	void applySorting();
	PosType findFileIndexByName(const std::wstring &filepath, const std::vector<std::wstring> &filelist);

	SortingStyle currentSorting = SortByName;

	lang::SignalBind filelistChangedBind;

	SizeType currentImageIndex;
	std::wstring currentFilePath;

	mutable Mutex mutex;
	std::vector<std::wstring> currentFileList;
};

TS_END_PACKAGE2()
