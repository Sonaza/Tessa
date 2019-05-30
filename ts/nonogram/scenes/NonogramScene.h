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
	std::unique_ptr<gameplay::NonogramPuzzle> puzzle;

	sf::Texture texture;
	sf::Sprite sprite;

};

TS_END_PACKAGE2()
