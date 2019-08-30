#include "Precompiled.h"
#include "ts/nonogram/scenes/NonogramScene.h"

#include "ts/tessa/file/ArchivistFilesystem.h"
#include "ts/tessa/file/FileUtils.h"
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
	file::ArchivistFilesystem &afs = getGigaton<file::ArchivistFilesystem>();
	
	const std::vector<std::string> list = afs.getFileList();
	for (const std::string &filename : list)
	{
		std::string ext = file::utils::getExtension(filename);
		if (ext == "jpg" || ext == "jpeg" || ext == "png")
		{
			std::string handle = file::utils::getBasename(filename, true);
			textures.push_back(rm.loadResource<resource::TextureResource>(handle, filename));
		}
	}

	music = rm.loadResource<resource::MusicResource>("bg_music", "test/sol.ogg", true);
	music->getResource()->setLoop(true);
	music->getResource()->play();
}

bool NonogramScene::handleEvent(const sf::Event event)
{
	switch (event.type)
	{
		case sf::Event::KeyPressed:
		{
			if (event.key.code == sf::Keyboard::Left)
			{
				swapTexture(-1);
				return true;
			}
			if (event.key.code == sf::Keyboard::Right)
			{
				swapTexture(1);
				return true;
			}
		}
		break;
	}

	return false;
}

void NonogramScene::swapTexture(int32 dir)
{
	currentIndex += dir;
	if (currentIndex < 0)
		currentIndex = (int32)(textures.size() - 1);
	if (currentIndex >= textures.size())
		currentIndex = 0;

	resource::TextureResource *texture = textures[currentIndex];

	if (texture && texture->isLoaded())
	{
		sf::Texture *tex = texture->getResource();
		sprite.setTexture(*tex, true);

		math::VC2U size = tex->getSize();
		float scale = 800.f / (float)size.y;

		sprite.setScale(scale, scale);

		system::WindowManager &wm = application->getManager<system::WindowManager>();
		math::VC2U windowSize = wm.getSize();
		sprite.setPosition(windowSize.x / 2.f, windowSize.y / 2.f);

		sprite.setOrigin(size.x / 2.f, size.y / 2.f);
	}
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
	else
	{
		swapTexture(0);
	}
}

void NonogramScene::render(sf::RenderWindow &renderWindow)
{
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
