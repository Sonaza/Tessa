#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

#include <map>

TS_DECLARE1(resource, SoundResource);
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

	virtual void update(const TimeSpan deltaTime);

	String getStats();

	Image *getCurrentImage() const;

	enum DisplayShaderTypes
	{
		DisplayShader_FreeImage,
		DisplayShader_Webm,
	};
	SharedPointer<sf::Shader> loadDisplayShader(DisplayShaderTypes type);

	lang::Signal<SizeType> currentImageChangedSignal;

private:
	void prepareShaders();
	std::map<DisplayShaderTypes, String> displayShaderFiles;

	void currentImageChanged(SizeType imageIndex);
	void updateCurrentImage();

	bool pendingImageUpdate = false;
	Clock imageChangedTimer;

	SizeType currentImageIndex = 0;
	uint32 currentImageHash = 0;

	typedef std::map<uint32, SharedPointer<Image>> ImageStorageList;
	ImageStorageList imageStorage;

	SharedPointer<sf::Texture> alphaCheckerPatternTexture;

	class BackgroundImageUnloader;
	ScopedPointer<BackgroundImageUnloader> backgroundUnloader;

	lang::SignalBind currentImageChangedBind;

	mutable Mutex mutex;
};

TS_END_PACKAGE2()
