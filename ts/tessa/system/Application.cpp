#include "Precompiled.h"
#include "ts/tessa/common/Debugging.h"
#include "ts/tessa/system/Application.h"

#include "ts/tessa/file/ArchivistFileSystem.h"
#include "ts/tessa/system/WindowManager.h"
#include "ts/tessa/resource/ResourceManager.h"
#include "ts/tessa/threading/ThreadManager.h"
#include "ts/tessa/threading/ThreadUtils.h"

#include "ts/tessa/resource/FontResource.h"

#include "ts/tessa/Config.h"

#include "ts/tessa/system/Gigaton.h"

TS_PACKAGE1(system)

Application::Application(Int32 argc, const char **argv)
	: _commando(argc, argv)
{
	TS_GIGATON_REGISTER_CLASS(this);
}

Application::~Application()
{
	TS_GIGATON_UNREGISTER_CLASS(this);
}

Int32 Application::start()
{
	if (!_config.parse(TS_CONFIG_FILE_NAME))
	{
		TS_LOG_ERROR("Unable to open options file. File: %s", TS_CONFIG_FILE_NAME);
	}

	common::Log::setLogFile(_config.getString("General.LogFile", TS_DEFAULT_LOG_FILE_NAME));

	if (!sf::Shader::isAvailable())
	{
		TS_LOG_ERROR("Shaders not available!");
		return 2;
	}

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
// 	setCurrentThreadPriority(threading::utils::ThreadPriority_High);

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

const Commando &Application::getCommando() const
{
	return _commando;
}

const ConfigReader &Application::getConfig() const
{
	return _config;
}

bool Application::initializeManagers()
{
	if (!createManagerInstance<threading::ThreadManager>())
		return false;

	if (!createManagerInstance<file::ArchivistFilesystem>())
		return false;

	file::ArchivistFilesystem &afs = getManager<file::ArchivistFilesystem>();
	if (!loadArchives(afs))
		return false;
	
	if (!createManagerInstance<resource::ResourceManager>())
		return false;

	resource::ResourceManager &rm = getManager<resource::ResourceManager>();
	debugFont = rm.loadResource<resource::FontResource>("_application_debug_font", "selawk.ttf", true);
	TS_ASSERT(debugFont != nullptr && debugFont->isLoaded() && "Loading debug font failed.");

	if (!createManagerInstance<system::WindowManager>())
		return false;

	system::WindowManager &wm = getManager<system::WindowManager>();
	if (!createWindow(wm))
	{
		TS_LOG_ERROR("Window creation failed.");
		return false;
	}

	return true;
}

void Application::deinitializeManagers()
{
	threading::ThreadManager &tm = getManager<threading::ThreadManager>();
	tm.clearTasks();

	destroyManagerInstance<system::WindowManager>();
	destroyManagerInstance<resource::ResourceManager>();
	destroyManagerInstance<file::ArchivistFilesystem>();
	destroyManagerInstance<threading::ThreadManager>();
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

		sf::Time deltaTime = deltaClock.restart();

		for (SystemManagersList::iterator it = systemManagers.begin(); it != systemManagers.end(); ++it)
		{
			it->second->update(deltaTime);
		}

		handleEvents();

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

	if (debugFont != nullptr && debugFont->isLoaded())
	{
		sf::Text fps;
		fps.setCharacterSize(20);
		fps.setFillColor(sf::Color::White);
		fps.setOutlineColor(sf::Color::Black);
		fps.setOutlineThickness(1.f);
		fps.setFont(*debugFont->getResource());
		fps.setString(TS_FMT("FPS %u", getCurrentFramerate()));

		renderWindow.draw(fps);
	}

	renderWindow.display();
}

TS_END_PACKAGE1()
