#pragma once

#include "ts/tessa/system/AbstractSceneBase.h"

TS_DECLARE2(app, viewer, ViewerManager);
TS_DECLARE2(app, viewer, Image);

TS_DECLARE_STRUCT1(system, WindowView);

TS_PACKAGE2(app, scenes)

class ImageViewerScene : public system::AbstractSceneBase
{
	TS_DECLARE_SCENE(ImageViewerScene);

public:
	ImageViewerScene(system::BaseApplication *application);
	virtual ~ImageViewerScene();

	virtual bool start();
	virtual void stop();

	virtual void loadResources(resource::ResourceManager &rm) override;

	virtual bool handleEvent(const sf::Event event) override;
	virtual void update(const TimeSpan deltaTime) override;

	virtual void renderApplication(sf::RenderTarget &renderTarget, const system::WindowView &view) override;
	virtual void renderInterface(sf::RenderTarget &renderTarget, const system::WindowView &view) override;

protected:
	void imageChanged(SharedPointer<viewer::Image> image);
	lang::SignalBind imageChangedBind;

	void filelistChanged(SizeType numFiles);
	lang::SignalBind filelistChangedBind;

	void screenResized(const math::VC2U &size);
	lang::SignalBind screenResizedBind;

	void filesDropped(const std::vector<system::DroppedFile> &files);
	lang::SignalBind filesDroppedBind;

	resource::FontResource *font = nullptr;

	float framePadding = 20.f;
	
	Clock clickTimer;
	Clock frameTimer;
	Clock changeTimer;

	bool updateImageInfo();

	SharedPointer<viewer::Image> currentImage;
	bool pendingImageInfo = true;
	math::VC2U imageSize;

	float defaultScale = 1.f;
	float targetDefaultScale = 1.f;

	float imageScale = 1.f;
	float targetImageScale = 1.f;

	math::VC2 positionOffset;
	math::VC2 targetPositionOffset;
	
	void enforceOversizeLimits(float scale);
	math::VC2 positionOversizeLimit;

	bool recursiveFileSearch = true;

	enum DisplayMode
	{
		Normal,
		Manga,
	};
	DisplayMode displayMode = Normal;

	float dragged = 0.f;

	//////////////////////

	bool displaySmooth = true;

	math::VC2I lastMousePosition;

	bool showManagerStatus = false;
	bool showSchedulerStatus = false;

	system::WindowManager *windowManager = nullptr;
	viewer::ViewerManager *viewerManager = nullptr;

	sf::Sprite sprite;
};

TS_END_PACKAGE2()
