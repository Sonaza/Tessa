#pragma once

#include "ts/tessa/system/AbstractSceneBase.h"

#include <chrono>

TS_DECLARE2(game, gameplay, NonogramPuzzle)

TS_PACKAGE2(game, scenes)

class NonogramScene : public system::AbstractSceneBase
{
public:
	NonogramScene(system::Application *application);
	virtual ~NonogramScene();

	virtual bool start();
	virtual void stop();

	virtual void loadResources(resource::ResourceManager &rm);

	virtual bool handleEvent(const sf::Event event);
	virtual void update(const sf::Time deltaTime);
	virtual void render(sf::RenderWindow &renderWindow);

protected:
	math::VC2 velocity = math::VC2::zero;
	math::VC2 gravity = math::VC2(0.f, 1000.f);
	sf::Clock timer;

	std::unique_ptr<gameplay::NonogramPuzzle> puzzle;

	void swapTexture(int32 dir);
	int32 currentIndex = 0;

	resource::MusicResource *music = nullptr;
	std::vector<resource::TextureResource *> textures;

	sf::Sprite sprite;
};

TS_END_PACKAGE2()
