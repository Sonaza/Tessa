#include "Precompiled.h"
#include "ts/nonogram/scenes/NonogramScene.h"

#include "ts/nonogram/gameplay/NonogramPuzzle.h"

TS_PACKAGE2(game, scenes)

NonogramScene::NonogramScene(system::Application *application)
	: AbstractSceneBase(application)
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

void NonogramScene::loadResources(resource::ResourceManager &rm)
{
	rm.loadResource<resource::TextureResource>("nepnep", "test/nepnep.jpg");
	rm.loadResource<resource::TextureResource>("2Gzjp1F", "test/2Gzjp1F.jpg");
	rm.loadResource<resource::TextureResource>("33zyzuxd0d911", "test/33zyzuxd0d911.jpg");
	rm.loadResource<resource::TextureResource>("47168900_p0_master1200", "test/47168900_p0_master1200.jpg");
	rm.loadResource<resource::TextureResource>("47168900_p1_master1200", "test/47168900_p1_master1200.jpg");
	rm.loadResource<resource::TextureResource>("47372029_p0", "test/47372029_p0.png");
	rm.loadResource<resource::TextureResource>("49561770_p0", "test/49561770_p0.png");
	rm.loadResource<resource::TextureResource>("4xvzjxz2hw211", "test/4xvzjxz2hw211.jpg");
	rm.loadResource<resource::TextureResource>("54638899_p0", "test/54638899_p0.png");
	rm.loadResource<resource::TextureResource>("55378833_p0", "test/55378833_p0.png");
	rm.loadResource<resource::TextureResource>("59335804_p0", "test/59335804_p0.png");
	rm.loadResource<resource::TextureResource>("61579330_p0", "test/61579330_p0.jpg");
	rm.loadResource<resource::TextureResource>("61827137_p0", "test/61827137_p0.png");
	rm.loadResource<resource::TextureResource>("656mwuonuh011", "test/656mwuonuh011.jpg");
	rm.loadResource<resource::TextureResource>("nepzume", "test/nepzume.png", true);
	rm.loadResource<resource::TextureResource>("67961452_p11", "test/67961452_p11.png");
	rm.loadResource<resource::TextureResource>("68599620_p0", "test/68599620_p0.png");
	rm.loadResource<resource::TextureResource>("68783222_p0", "test/68783222_p0.png");
	rm.loadResource<resource::TextureResource>("68783222_p1", "test/68783222_p1.png");
	rm.loadResource<resource::TextureResource>("69103464_p0", "test/69103464_p0.png");
	rm.loadResource<resource::TextureResource>("a6wmhr23h3e11", "test/a6wmhr23h3e11.jpg");
	rm.loadResource<resource::TextureResource>("abitiffy", "test/abitiffy.png");
	rm.loadResource<resource::TextureResource>("compa", "test/compa.png");
	rm.loadResource<resource::TextureResource>("iffy", "test/iffy.jpg");
	rm.loadResource<resource::TextureResource>("stick", "test/stick.jpg");
	rm.loadResource<resource::TextureResource>("uzume", "test/uzume.jpg");

	rm.loadResource<resource::FontResource>("calibri", "calibri.ttf");

	texture = rm.getResource<resource::TextureResource>("nepzume");
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
		system::WindowManager &wm = application->getManager<system::WindowManager>();
		math::VC2U windowSize = wm.getSize();

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

		if (timer.getElapsedTime() > sf::milliseconds(3000))
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

		sprite.setScale(0.5f, 0.5f);

		system::WindowManager &wm = application->getManager<system::WindowManager>();
		math::VC2U windowSize = wm.getSize();
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
