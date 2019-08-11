#pragma once

#include "ts/tessa/system/AbstractSceneBase.h"

TS_PACKAGE2(app, scenes)

class ImageViewerScene : public system::AbstractSceneBase
{
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
	void loadImage();

// 	void swapTexture(Int32 dir);
	Int32 currentIndex = 0;

	sf::Shader shader;

	sf::Texture moireTexture;
	math::VC2U texSize;

	sf::RenderTexture blurTexture;

	sf::Sprite sprite;
};

TS_END_PACKAGE2()
