#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

#include "ts/tessa/lang/Signal.h"

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
	SharedPointer<sf::Shader> getDisplayShader(DisplayShaderTypes type);
	bool loadDisplayShader(DisplayShaderTypes type, const std::string &handle, const std::string &filepath);

	lang::Signal<SizeType> currentImageChangedSignal;

private:
	std::map<DisplayShaderTypes, resource::ShaderResource *> displayShaders;

	void currentImageChanged(SizeType imageIndex);

	SizeType currentImageIndex = 0;
	Uint32 currentImageHash = 0;

	typedef std::map<Uint32, UniquePointer<Image>> ImageStorageList;
	ImageStorageList imageStorage;

// 	std::vector<Uint32> currentlyActiveImages;

	lang::SignalBind currentImageChangedBind;


	mutable std::mutex mutex;
};

TS_END_PACKAGE2()
