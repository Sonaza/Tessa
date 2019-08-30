#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

TS_PACKAGE2(app, viewer)

struct ImageEntry 
{
	String filepath;
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
	const String &getCurrentFilepath() const;

	void nextImage();
	void previousImage();

	void jumpToImage(SizeType index);
	void jumpToImageByFilename(const String &filename);
	SizeType getNumImages() const;

	enum SortingStyle
	{
		SortByName,
		SortByExtension,
	};
	void setSorting(SortingStyle sorting);

	const std::vector<String> &getCurrentSortedFileList() const;

	// Retrieves a list of entries for buffering. 
	const std::vector<ImageEntry> getListSliceForBuffering(SizeType numForward, SizeType numBackward);

	lang::Signal<SizeType> currentImageChangedSignal;

private:
	void updateFileList();

	void applySorting();
	PosType findFileIndexByName(const String &filepath, const std::vector<String> &filelist);

	SortingStyle currentSorting = SortByName;

	lang::SignalBind filelistChangedBind;

	SizeType currentImageIndex;
	String currentFilePath;

	mutable Mutex mutex;
	std::vector<String> currentFileList;
};

TS_END_PACKAGE2()
