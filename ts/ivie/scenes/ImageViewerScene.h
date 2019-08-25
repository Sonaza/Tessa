#pragma once

#include "ts/tessa/system/AbstractSceneBase.h"
#include "ts/tessa/lang/Signal.h"

TS_DECLARE2(app, viewer, ViewerStateManager);

TS_PACKAGE2(app, scenes)

class ImageViewerScene : public system::AbstractSceneBase
{
	TS_DECLARE_SCENE(ImageViewerScene);

public:
	ImageViewerScene(system::BaseApplication *application);
	virtual ~ImageViewerScene();

	virtual bool start();
	virtual void stop();

	virtual void loadResources(resource::ResourceManager &rm);

	virtual bool handleEvent(const sf::Event event);
	virtual void update(const TimeSpan deltaTime);
	virtual void render(sf::RenderWindow &renderWindow);

protected:
	void imageChanged(SizeType imageIndex);
	lang::SignalBind imageChangedBind;

	resource::FontResource *font = nullptr;

	Clock frameTimer;

	float imageScale = 1.f;
	float targetImageScale = 1.f;

	math::VC2 positionOffset;
	math::VC2 targetPositionOffset;

	math::VC2I lastMousePosition;

	viewer::ViewerStateManager *viewerStateManager = nullptr;

	sf::Sprite sprite;
};

TS_END_PACKAGE2()
