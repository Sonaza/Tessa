#pragma once

#include "ts/tessa/system/AbstractSceneBase.h"

TS_DECLARE2(app, viewer, ViewerStateManager);

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
	void imageChanged(SizeType imageIndex);
	lang::SignalBind imageChangedBind;

	resource::FontResource *font = nullptr;

	Clock clickTimer;

	bool updateImageInfo = true;
	math::VC2U imageSize;
	float defaultScale = 1.f;
	float dragged = 0.f;

	Clock frameTimer;

	float imageScale = 1.f;
	float targetImageScale = 1.f;

	math::VC2 positionOffset;
	math::VC2 targetPositionOffset;

	math::VC2I lastMousePosition;

	bool showManagerStatus = false;
	bool showSchedulerStatus = false;

	viewer::ViewerStateManager *viewerStateManager = nullptr;

	sf::Sprite sprite;
};

TS_END_PACKAGE2()
