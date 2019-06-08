#include "Precompiled.h"
#include "ts/tessa/system/Application.h"

#include "ts/tessa/system/SystemManagerBase.h"
#include "ts/tessa/system/SceneBase.h"
#include "ts/tessa/system/ThreadPool.h"
#include "ts/tessa/system/WindowManager.h"

#include "ts/tessa/resource/ResourceManager.h"
#include "ts/tessa/resource/FontResource.h"

TS_PACKAGE1(system)

Application::Application(Int32 argc, const char **argv)
	: commando(argc, argv)
{
}

Application::~Application()
{
}

Int32 Application::start()
{
	if (!initialize())
	{
		TS_LOG_ERROR("Application::initialize() failed, cannot proceed.\n");
		return 1;
	}
	
	if (sceneInitialize())
	{
		mainloop();
	}
	else
	{
		TS_LOG_ERROR("Scene initialize failed!\n");
	}

	deinitialize();

	return 0;
}

bool Application::initialize()
{
	if (!initializeManagers())
		return false;

	return true;
}

void Application::deinitialize()
{
	if (currentScene != nullptr)
		currentScene->internalStop();

	pendingScene.reset();
	currentScene.reset();

	deinitializeManagers();
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

bool Application::initializeManagers()
{
	threadPool = std::make_unique<system::ThreadPool>(system::ThreadPool::numHardwareThreads());
	if (threadPool == nullptr) return false;
// 	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(false, threadPool);

	if (!createManagerInstance<resource::ResourceManager>())
		return false;

	resource::ResourceManager &rm = getManager<resource::ResourceManager>();
	rm.loadResource<resource::FontResource>("debugfont", "arial.ttf");

	if (!createManagerInstance<system::WindowManager>())
		return false;

	system::WindowManager &wm = getManager<system::WindowManager>();
	wm.create(math::VC2U(1600, 1000), "Nonograms Testing");

	return true;
}

void Application::deinitializeManagers()
{
	threadPool.reset();

	destroyManagerInstance<system::WindowManager>();
	destroyManagerInstance<resource::ResourceManager>();
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
				TS_LOG_ERROR("Pending scene start failed. Scene will not be loaded.");
				pendingScene.reset();
			}

			if (currentScene != nullptr)
			{
				currentScene->internalStop();
				currentScene.reset();
			}

			currentScene = std::move(pendingScene);
			currentScene->loadResources(getManager<resource::ResourceManager>());
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
	system::WindowManager &windowManager = getManager<system::WindowManager>();
	if (!windowManager.isOpen())
		return;

	sf::Event event;
	while (windowManager.pollEvent(event))
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
	
	system::WindowManager &windowManager = getManager<system::WindowManager>();
	if (!windowManager.isOpen())
		return;

	sf::RenderWindow &renderWindow = windowManager.getRenderWindow();
	renderWindow.clear();
	
	// Game view step
	windowManager.useGameView();

	currentScene->render(renderWindow);

	// Interface view step
	windowManager.useInterfaceView();

// 	resource::ResourceManager &resourceManager = getManager<resource::ResourceManager>();
// 	resource::FontResource *font = resourceManager.getResource<resource::FontResource>("debugfont");
// 	if (font != nullptr && font->isLoaded())
	{
		sf::Text fps;
		fps.setCharacterSize(20);
		fps.setFillColor(sf::Color::White);
		fps.setOutlineColor(sf::Color::Black);
		fps.setOutlineThickness(1.f);
// 		fps.setFont(*font->getResource());
		fps.setString(TS_FMT("FPS %u", getCurrentFramerate()));

		renderWindow.draw(fps);
	}

	renderWindow.display();
}

TS_END_PACKAGE1()
