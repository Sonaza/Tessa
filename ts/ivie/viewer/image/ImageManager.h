#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

#include "ts/tessa/lang/Signal.h"

TS_DECLARE2(app, viewer, Image);

TS_PACKAGE2(app, viewer)

class ImageManager : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(app::viewer::ImageManager);

public:
	ImageManager();
	~ImageManager();

	virtual bool initialize();
	virtual void deinitialize();

	std::wstring getStats();

	Image *getCurrentImage();

	lang::Signal<SizeType> currentImageChangedSignal;

private:
	void currentImageChanged(SizeType imageIndex);

	SizeType currentImageIndex = 0;
	Uint32 currentImageHash = 0;

	typedef std::map<Uint32, UniquePointer<Image>> ImageStorageList;
	ImageStorageList imageStorage;

// 	std::vector<Uint32> currentlyActiveImages;

	lang::SignalBind currentImageChangedBind;

	mutable std::mutex mutex;
// 	std::vector<std::wstring> currentFileList;
};

TS_END_PACKAGE2()
