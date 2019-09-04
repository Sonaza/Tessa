#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"
#include "ts/tessa/thread/ThreadScheduler.h"

#include "ts/tessa/file/FileList.h"

TS_DECLARE2(app, viewer, Image);

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

class ViewerManager : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(app::viewer::ViewerManager);

public:
	ViewerManager();
	~ViewerManager();

	virtual bool initialize();
	virtual void deinitialize();

	void update(const TimeSpan deltaTime);

	void setFilepath(const String &filepath);
	const String &getFilepath() const;

	void setRecursiveScan(bool recursiveEnabled);

	enum SortingStyle
	{
		SortByName,
		SortByExtension,
	};
	void setSorting(SortingStyle sorting);

	//////////////////////////////////////////////////////
	// Image state
	
	void jumpToImage(SizeType index);
	void jumpToImageByFilename(const String &filename);
	void nextImage();
	void previousImage();
	void changeImage(int32 direction);

	SizeType getCurrentImageIndex() const;
	SizeType getNumImages() const;

	const String &getCurrentFilepath() const;

	SharedPointer<Image> getCurrentImage() const;

	enum DisplayShaderTypes
	{
		DisplayShader_FreeImage,
		DisplayShader_Webm,
	};
	SharedPointer<sf::Shader> loadDisplayShader(DisplayShaderTypes type);

	String getStats();

	// When file list changes, parameter is number of files
	lang::Signal<SizeType> filelistChangedSignal;

	// When current image changes, parameter is SharedPointer of the image (nullptr of none)
	lang::Signal<SharedPointer<Image>> currentImageChangedSignal;

private:
	std::atomic_bool quitting = false;

	void prepareShaders();
	std::map<DisplayShaderTypes, String> displayShaderFiles;
	SharedPointer<sf::Texture> alphaCheckerPatternTexture;

	void applySorting(std::vector<String> &filelist);
	void ensureImageIndex();

	bool isExtensionAllowed(const String &filename);
	bool updateFilelist(const String directoryPath, bool ensureIndex);

	const std::vector<ImageEntry> getListSliceForBuffering(SizeType numForward, SizeType numBackward);

	PosType findFileIndexByName(const String &filepath, const std::vector<String> &filelist);

	std::vector<String> allowedExtensions;
	SortingStyle currentSorting = SortByName;

	file::FileListStyle scanStyle = file::FileListStyle_Files_Recursive;

	String currentDirectoryPath;

	struct DisplayState
	{
		SizeType imageIndex = 0;
		String filepath;
	};
	DisplayState current;
	SharedPointer<Image> currentImage;

	bool pendingImageUpdate = true;
	PosType pendingImageIndex = 0;

// 	struct ImageFile
// 	{
// 		String filepath;
// 		int64 lastModifiedTime;
// 	};
	std::vector<String> currentFileList;

	void updateCurrentImage();

	typedef std::map<uint32, SharedPointer<Image>> ImageStorageList;
	ImageStorageList imageStorage;

	class BackgroundImageUnloader;
	ScopedPointer<BackgroundImageUnloader> backgroundUnloader;

	thread::SchedulerTaskId scannerTaskId = thread::InvalidTaskId;

	thread::ThreadScheduler *threadScheduler = nullptr;
	mutable Mutex mutex;
};

TS_END_PACKAGE2()
