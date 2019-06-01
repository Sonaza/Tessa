#include "Precompiled.h"
#include "ts/tessa/system/Application.h"

#include "ts/tessa/system/SceneBase.h"
#include "ts/tessa/system/ThreadPool.h"
#include "ts/tessa/system/Window.h"

#include "ts/tessa/resource/ResourceManager.h"
#include "ts/tessa/resource/FontResource.h"

TS_PACKAGE1(system)

Application::Application()
{
}

Application::~Application()
{
}

void Application::initialize()
{
	initializeManagers();
	initializeWindow();
}

void Application::deinitialize()
{
	if (currentScene != nullptr)
		currentScene->internalStop();

	if (window)
	{
		window->close();
		window.reset();
	}

	pendingScene.reset();
	currentScene.reset();
}

void Application::launch()
{
	initialize();

	if (sceneInitialize())
	{
		mainloop();
	}
	else
	{
		TS_PRINTF("Scene initialize failed!\n");
	}

	deinitialize();
}

void Application::setFramerateLimit(SizeType framerateLimit)
{
	if (framerateLimit == 0)
	{
		targetFrameTime = sf::milliseconds(16);
		return;
	}

	targetFrameTime = sf::milliseconds(1000 / framerateLimit);
}

SizeType Application::getCurrentFramerate() const
{
	return currentFramerate;
}

void Application::initializeManagers()
{
	threadPool = std::make_unique<ThreadPool>(ThreadPool::numHardwareThreads());

	resourceManager = std::make_shared<resource::ResourceManager>(getApplicationPtr());

	resourceManager->loadResource<resource::FontResource>("debugfont", "arial.ttf");
}

void Application::initializeWindow()
{
	window = std::make_unique<Window>(getApplicationPtr());
	if (window != nullptr)
	{
		window->create(math::VC2U(1600, 1000), "Nonograms Testing");
	}
}

void Application::mainloop()
{
	const sf::Time fixedDeltaTime = sf::milliseconds(16);

	sf::Time deltaAccumulator;

	SizeType frameCounter = 0;
	sf::Clock framerateClock;

	sf::Clock deltaClock;
	sf::Clock loopTimer;
	while (applicationRunning)
	{
		loopTimer.restart();

		if (pendingScene != nullptr)
		{
			bool successfulStart = pendingScene->internalStart();
			if (!successfulStart)
			{
				TS_PRINTF("Pending scene start failed. Scene will not be loaded.");
				pendingScene.reset();
			}

			if (currentScene != nullptr)
			{
				currentScene->internalStop();
				currentScene.reset();
			}

			currentScene = std::move(pendingScene);
			currentScene->loadResources(resourceManager);
		}

		handleEvents();

		sf::Time deltaTime = deltaClock.restart();
		deltaAccumulator += deltaTime;

		while (deltaAccumulator >= fixedDeltaTime)
		{
			handleUpdate(fixedDeltaTime);
			deltaAccumulator -= fixedDeltaTime;
		}

		handleRendering();

		sf::sleep(targetFrameTime - loopTimer.getElapsedTime());

		frameCounter++;
		if (framerateClock.getElapsedTime() > sf::milliseconds(500))
		{
			float elapsedTime = framerateClock.restart().asSeconds();
			currentFramerate = (SizeType)(frameCounter / elapsedTime);
			frameCounter = 0;
		}
	}

	if (currentScene != nullptr)
	{
		currentScene->stop();
	}
}

void Application::handleEvents()
{
	if (window == nullptr || !window->isOpen())
		return;

	sf::Event event;
	while (window->pollEvent(event))
	{
		if (currentScene != nullptr)
		{
			// If event is handled by scene the rest can be skipped
			if (currentScene->handleEvent(event))
				continue;
		}

		switch (event.type)
		{
			case sf::Event::Closed:
			{
				applicationRunning = false;
			}
			break;

			case sf::Event::KeyPressed:
			{
				if (event.key.code == sf::Keyboard::Escape)
				{
					applicationRunning = false;
				}
			}
			break;
		}
	}
}

void Application::handleUpdate(const sf::Time deltaTime)
{
	if (currentScene == nullptr)
		return;

	currentScene->update(deltaTime);
}

void Application::handleRendering()
{
	if (currentScene == nullptr)
		return;
	
	if (window == nullptr || !window->isOpen())
		return;

	sf::RenderWindow &renderWindow = window->getRenderWindow();
	renderWindow.clear();
	
	// Game view step
	window->useGameView();

	currentScene->render(renderWindow);

	// Interface view step
	window->useInterfaceView();

	std::shared_ptr<resource::FontResource> font = resourceManager->getResource<resource::FontResource>("debugfont");
	if (font)
	{
		char buffer[15];
		sprintf_s(buffer, sizeof(buffer), "FPS %u", getCurrentFramerate());

		sf::Text fps;
		fps.setCharacterSize(20);
		fps.setOutlineColor(sf::Color::Black);
		fps.setOutlineThickness(1.f);
		fps.setFont(*font->getResource());
		fps.setString(buffer);

		renderWindow.draw(fps);
	}

	renderWindow.display();
}

TS_END_PACKAGE1()
