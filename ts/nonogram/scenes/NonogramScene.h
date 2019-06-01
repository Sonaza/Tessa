#pragma once

#include "ts/tessa/system/SceneBase.h"

TS_DECLARE2(game, gameplay, NonogramPuzzle)

TS_PACKAGE2(game, scenes)

class NonogramScene : public system::SceneBase
{
public:
	NonogramScene(std::shared_ptr<system::Application> application);
	virtual ~NonogramScene();

	virtual bool start();
	virtual void stop();

	virtual void loadResources(std::shared_ptr<resource::ResourceManager> rm);

	virtual bool handleEvent(const sf::Event event);
	virtual void update(const sf::Time deltaTime);
	virtual void render(sf::RenderWindow &renderWindow);

protected:
	math::VC2 velocity = math::VC2::zero;
	math::VC2 gravity = math::VC2(0.f, 1000.f);
	sf::Clock timer;

	std::unique_ptr<gameplay::NonogramPuzzle> puzzle;

	std::shared_ptr<resource::TextureResource> texture;

	sf::Sprite sprite;
};

TS_END_PACKAGE2()
