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

	std::wstring getStats();

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
	std::map<DisplayShaderTypes, std::string> displayShaderFiles;

	void currentImageChanged(SizeType imageIndex);

	SizeType currentImageIndex = 0;
	Uint32 currentImageHash = 0;

	typedef std::map<Uint32, UniquePointer<Image>> ImageStorageList;
	ImageStorageList imageStorage;

	SharedPointer<sf::Texture> alphaCheckerPatternTexture;

	lang::SignalBind currentImageChangedBind;

	mutable std::mutex mutex;
};

TS_END_PACKAGE2()
