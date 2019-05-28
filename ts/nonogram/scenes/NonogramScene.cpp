#include "Precompiled.h"
#include "ts/nonogram/scenes/NonogramScene.h"

#include "ts/nonogram/gameplay/NonogramPuzzle.h"

TS_PACKAGE2(game, scenes)

NonogramScene::NonogramScene(std::shared_ptr<system::Application> application)
	: SceneBase(application)
{
}

NonogramScene::~NonogramScene()
{
}

bool NonogramScene::start()
{
	puzzle.reset(new gameplay::NonogramPuzzle(10, 10));
	return true;
}

void NonogramScene::stop()
{

}

void NonogramScene::loadResources()
{
	texture.loadFromFile("picture2.jpg");
	sprite.setTexture(texture);
}

bool NonogramScene::handleEvent(const sf::Event event)
{
	return false;
}

void NonogramScene::update(const sf::Time deltaTime)
{

}

void NonogramScene::render(sf::RenderWindow &renderWindow)
{
	renderWindow.draw(sprite);

	{
		float squareSize = 50.f;

		sf::Vector2u gridSize = puzzle->getSize();
		sf::Vector2f puzzleSize(gridSize.x * squareSize, gridSize.y * squareSize);

		sf::VertexArray puzzleArray = puzzle->getVertexArray(squareSize);

		sf::Transform transform;
		transform.transformPoint(1600.f / 2.f - puzzleSize.x / 2.f, 1000.f / 2.f - puzzleSize.y / 2.f);

		sf::RenderStates states;
		states.transform = transform;
		renderWindow.draw(puzzleArray, states);
	}
}

TS_END_PACKAGE2()
