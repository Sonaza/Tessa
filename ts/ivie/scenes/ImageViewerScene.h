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

	resource::FontResource *font;

	lang::SignalBind imageChangedBind;

// 	void swapTexture(Int32 dir);
	Int32 currentIndex = 0;

	Clock frameTimer;

	sf::Shader shader;

	sf::Texture imageTexture;
	math::VC2U texSize;

	std::mutex textureMutex;

	viewer::ViewerStateManager *viewerStateManager = nullptr;

	sf::Sprite sprite;
};

TS_END_PACKAGE2()
