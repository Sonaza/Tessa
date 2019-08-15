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

	void nextImage();
	void previousImage();

	void jumpToImage(SizeType index);
	SizeType getNumImages() const;

private:
	void updateFileList();

	lang::SignalBind filelistChangedBind;

	SizeType currentFileIndex;
	std::vector<std::wstring> currentFileList;
};

TS_END_PACKAGE2()
