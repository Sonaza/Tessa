#pragma once

#include "ts/engine/system/AbstractManagerBase.h"
#include "ts/thread/ThreadScheduler.h"

#include "ts/file/FileList.h"
#include "ts/file/FileWatcher.h"

#include "ts/ivie/image/Image.h"

TS_PACKAGE2(app, viewer)

struct ImageEntry 
{
	String filepath;
	SizeType index;

	enum BufferingDirection
	{
		Buffering_Forwards,
		Buffering_Backwards,
	};
	BufferingDirection buffering;
};

enum SortingStyle
{
	SortingStyle_ByName,
	SortingStyle_ByExtension,
// 	SortingStyle_ByLastModified,

	SortingStyle_NumOptions,
};

class ViewerManager : public engine::system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(app::viewer::ViewerManager);

public:
	ViewerManager();
	~ViewerManager();

	virtual bool initialize() override;
	virtual void deinitialize() override;

	virtual void update(const TimeSpan deltaTime) override;

	void setViewerPath(const String &filepath);
	const String &getViewerPath() const;

	bool getIsRecursiveScan() const;
	void setRecursiveScan(bool recursiveEnabled, bool immediateRescan = true);

	void setSorting(SortingStyle sorting);
	SortingStyle getSorting() const;

	//////////////////////////////////////////////////////
	// Image state
	
	void jumpToImage(SizeType index);
	void jumpToImageByFilename(const String &filename);
	void jumpToImageByDirectory(const String &directory);
	void nextImage();
	void previousImage();
	void changeImage(int32 direction);

	bool deleteCurrentImage();
	bool rotateCurrentImage(image::Image::RotateDirection direction);

	SizeType getCurrentImageIndex() const;
	SizeType getNumImages() const;

	bool isScanningFiles() const;
	bool isFirstScanComplete() const;

	const String getCurrentFilepath(bool absolute = true) const;

	SharedPointer<image::Image> getCurrentImage() const;

	enum DisplayShaderTypes
	{
		DisplayShader_FreeImage,
		DisplayShader_Webm,
	};
	SharedPointer<resource::ShaderResource> loadDisplayShader(DisplayShaderTypes type);

	String getStats();

	// When file list changes, parameter is number of files
	lang::Signal<SizeType> filelistChangedSignal;

	// When current image changes, parameter is SharedPointer of the image (nullptr of none)
	lang::Signal<SharedPointer<image::Image>> imageChangedSignal;

private:
	static std::atomic_bool quitting;
	mutable Mutex mutex;

	void prepareShaders();
	std::map<DisplayShaderTypes, String> displayShaderFiles;
	SharedPointer<sf::Texture> alphaCheckerPatternTexture;

	void applySorting(std::vector<String> &filelist);
	void ensureImageIndex();

	enum IndexingAction
	{
		IndexingAction_DoNothing,       // keeps current index
		IndexingAction_KeepCurrentFile, // attempts to find current file in the new list, else resets to previous image
		IndexingAction_Reset,           // resets index to zero (first image)
	};

	bool isExtensionAllowed(const String &filename);
	bool updateFilelist(const String directoryPath,
		bool allowFullRecursive, IndexingAction indexingAction);

	bool firstScanComplete = false;
	std::atomic_bool scanningFiles;

	const std::vector<ImageEntry> getListSliceForBuffering(SizeType numForward, SizeType numBackward);

	PosType findFileIndexByName(const String &filepath, const std::vector<String> &filelist);

	file::FileListStyle scanStyle = file::FileListStyle_Files_Recursive;

	String currentDirectoryPath;
	uint32 currentDirectoryPathHash = 0;

	void resetFileWatcher(bool recursive);
	void watchNotify(const std::vector<file::FileNotifyEvent> &notifyEvent);
	file::FileWatcher fileWatcher;
	lang::SignalBind watchNotifyBind;

	static const SizeType INVALID_IMAGE_INDEX = ~0U;
	struct DisplayState
	{
		SizeType imageIndex = INVALID_IMAGE_INDEX;
		uint32 directoryHash = 0;
		String filepath;
	};
	DisplayState current;

	SharedPointer<image::Image> currentImage;

	// Sets pending image to given index on the currentFileList.
	// If index is INVALID_INDEX the pending image will be cleared instead.
	void setPendingImage(SizeType imageIndex);
	DisplayState pending;

	bool pendingImageUpdate = true;

	struct ImageFile
	{
		String filepath;
		int64 lastModifiedTime;
	};
	std::vector<String> currentFileList;
	SortingStyle currentSortingStyle = SortingStyle_ByName;

	void updateCurrentImage(SizeType previousDirectoryHash, SizeType previousImageIndex);

	typedef std::map<uint32, SharedPointer<image::Image>> ImageStorageList;
	ImageStorageList imageStorage;
	std::vector<uint32> lastActiveImages;

	class BackgroundImageUnloader;
	ScopedPointer<BackgroundImageUnloader> backgroundUnloader;

	std::vector<String> allowedExtensions;

	thread::SchedulerTaskId scannerTaskId = thread::InvalidTaskId;

	thread::ThreadScheduler *threadScheduler = nullptr;
};

TS_END_PACKAGE2()
