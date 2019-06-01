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

void NonogramScene::loadResources(std::shared_ptr<resource::ResourceManager> rm)
{
	rm->loadResource<resource::TextureResource>("nepnep", "test/stick.jpg");
	rm->loadResource<resource::TextureResource>("picture", "test/stick.jpg");

	texture = rm->getResource<resource::TextureResource>("picture");
	if (texture)
	{
		if (texture->isLoaded())
		{
			TS_PRINTF("Texture is already loaded in loadResources!\n");
		}
		else
		{
			TS_PRINTF("Texture not loaded in loadResources!\n");
		}
	}
	else
	{
		TS_PRINTF("Loading texture failed entirely!\n");
	}
}

bool NonogramScene::handleEvent(const sf::Event event)
{
	return false;
}

void NonogramScene::update(const sf::Time deltaTime)
{
	if (sprite.getTexture() != nullptr)
	{
		math::VC2U windowSize = application->getWindowPtr()->getSize();

		sf::Vector2u size = sprite.getTexture()->getSize();

		velocity += gravity * deltaTime.asSeconds();

		sf::Vector2f pos = sprite.getPosition();

		pos.x += velocity.x * deltaTime.asSeconds();
		pos.y += velocity.y * deltaTime.asSeconds();
		sprite.setPosition(pos);

		sf::FloatRect b = sprite.getGlobalBounds();
		
		if (b.left < 0.f || (b.left + b.width) > (float)windowSize.x)
		{
			velocity.x *= -0.8f;
			pos.x = math::clamp(pos.x, b.width / 2.f, (float)windowSize.x - b.width / 2.f);
		}

		if (b.top < 0.f || (b.top + b.height) > (float)windowSize.y)
		{
			velocity.y *= -0.8f;
			pos.y = math::clamp(pos.y, b.height / 2.f, (float)windowSize.y - b.height / 2.f);
		}
		sprite.setPosition(pos);

		if (timer.getElapsedTime() > sf::milliseconds(5250))
		{
			gravity *= -1.f;
			timer.restart();
		}
	}
}

void NonogramScene::render(sf::RenderWindow &renderWindow)
{
	if (texture && texture->isLoaded() && sprite.getTexture() == nullptr)
	{
		TS_PRINTF("Texture is now set!\n");
		sprite.setTexture(*texture->getResource());

		math::VC2U windowSize = application->getWindowPtr()->getSize();
		sprite.setPosition(windowSize.x / 2.f, windowSize.y / 2.f);

		sf::Vector2u tsize = sprite.getTexture()->getSize();
		sprite.setOrigin(tsize.x / 2.f, tsize.y / 2.f);
	}

// 	sf::Vector2i mp = sf::Mouse::getPosition(renderWindow);
// 	sprite.setPosition((float)mp.x, (float)mp.y);
	renderWindow.draw(sprite);

	/*{
		float squareSize = 50.f;

		sf::Vector2u gridSize = puzzle->getSize();
		sf::Vector2f puzzleSize(gridSize.x * squareSize, gridSize.y * squareSize);

		sf::VertexArray puzzleArray = puzzle->getVertexArray(squareSize);

		sf::Transform transform;
		transform.transformPoint(1600.f / 2.f - puzzleSize.x / 2.f, 1000.f / 2.f - puzzleSize.y / 2.f);

		sf::RenderStates states;
		states.transform = transform;
		renderWindow.draw(puzzleArray, states);
	}*/
}

TS_END_PACKAGE2()
