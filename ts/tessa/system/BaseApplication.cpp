#include "Precompiled.h"
#include "ts/tessa/common/Debugging.h"
#include "ts/tessa/system/BaseApplication.h"

#include "ts/tessa/file/ArchivistFileSystem.h"
#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/system/WindowManager.h"
#include "ts/tessa/system/WindowViewManager.h"
#include "ts/tessa/resource/ResourceManager.h"

#include "ts/tessa/thread/Thread.h"
#include "ts/tessa/thread/ThreadScheduler.h"
#include "ts/tessa/thread/ThreadUtils.h"

#include "ts/tessa/resource/FontResource.h"

#include "ts/tessa/Config.h"

#include "ts/tessa/system/Gigaton.h"

TS_PACKAGE1(system)

BaseApplication::BaseApplication(Int32 argc, const char **argv)
	: gigaton(TS_GET_GIGATON())
	, _commando(argc, argv)
{
	gigaton.registerClass(this);
}

BaseApplication::BaseApplication(Int32 argc, const wchar_t **argv)
	: gigaton(TS_GET_GIGATON())
	, _commando(argc, argv)
{
	gigaton.registerClass(this);
}

BaseApplication::~BaseApplication()
{
	gigaton.unregisterClass(this);
}

Int32 BaseApplication::launch()
{
	initializeConfigDefaults(_config);

	if (!_config.open(TS_CONFIG_FILE_NAME))
	{
		if (!file::utils::exists(TS_CONFIG_FILE_NAME))
		{
			// Config doesn't exist, create a default.
			_config.save(TS_CONFIG_FILE_NAME);
			TS_LOG_WARNING("Unable to open config file. File: %s. Created a new file with application defaults.", TS_CONFIG_FILE_NAME);
		}
		else
		{
			TS_LOG_WARNING("Unable to open config file. File: %s. File exists but likely has syntax errors.", TS_CONFIG_FILE_NAME);
		}
	}

	common::Log::setLogFile(_config.getString("General.LogFile", TS_DEFAULT_LOG_FILE_NAME));

	if (sf::Shader::isAvailable() == false)
	{
		TS_LOG_ERROR("Shaders not available!");
		return 2;
	}

	if (initialize() == false)
	{
		TS_LOG_ERROR("BaseApplication::initialize() failed, cannot proceed.\n");
		return 1;
	}

	if (start() == false)
	{
		TS_LOG_ERROR("BaseApplication::start() failed, cannot proceed.\n");
		deinitialize();
		return -1;
	}
	
	if (initializeScene())
	{
		mainloop();
	}
	else
	{
		TS_LOG_ERROR("Scene initialize failed!\n");
	}

	stop();

	deinitialize();

	return 0;
}

bool BaseApplication::initialize()
{
	if (!createSystemManagers())
		return false;

	if (!createApplicationManagers())
		return false;

	TS_ASSERT(managerInstances.size() == managerInstancingOrder.size());
	for (auto it = managerInstancingOrder.begin(); it != managerInstancingOrder.end(); ++it)
	{
		const std::type_index &typeIndex = *it;
		if (managerInstances[typeIndex]->initialize())
		{
			managerInstances[typeIndex]->initialized = true;
		}
		else
		{
			TS_LOG_ERROR("Initializing a manager instance failed. Manager type: %s", managerInstances[typeIndex]->getTypeName());
			return false;
		}
	}

	file::ArchivistFilesystem &afs = getManager<file::ArchivistFilesystem>();
	if (!loadArchives(afs))
		return false;

	system::WindowManager &wm = getManager<system::WindowManager>();
	if (!createWindow(wm))
	{
		TS_LOG_ERROR("Window creation failed.");
		return false;
	}

	return true;
}

void BaseApplication::deinitialize()
{
	if (currentScene != nullptr)
		currentScene->internalStop();

	pendingScene.reset();
	currentScene.reset();

	destroyManagerInstances();
}

void BaseApplication::setFramerateLimit(SizeType framerateLimit)
{
	if (framerateLimit == 0)
	{
		targetFrameTime = TimeSpan::fromMilliseconds(16);
		return;
	}

	targetFrameTime = TimeSpan::fromMilliseconds(1000 / framerateLimit);
}

SizeType BaseApplication::getCurrentFramerate() const
{
	return currentFramerate;
}

const Commando &BaseApplication::getCommando() const
{
	return _commando;
}

ConfigReader &BaseApplication::getConfig()
{
	return _config;
}

const ConfigReader &BaseApplication::getConfig() const
{
	return _config;
}

sf::Font &BaseApplication::getDebugFont()
{
	return *debugFont->getResource();
}

bool BaseApplication::createSystemManagers()
{
	if (!createManagerInstance<thread::ThreadScheduler>())
		return false;

	if (!createManagerInstance<file::ArchivistFilesystem>())
		return false;
	
	if (!createManagerInstance<resource::ResourceManager>())
		return false;

	if (!createManagerInstance<system::WindowManager>())
		return false;

	if (!createManagerInstance<system::WindowViewManager>())
		return false;

	return true;
}

void BaseApplication::destroyManagerInstances()
{
	TS_ASSERT(managerInstances.size() == managerInstancingOrder.size());
	for (auto it = managerInstancingOrder.rbegin(); it != managerInstancingOrder.rend(); ++it)
	{
		const std::type_index &typeIndex = *it;
		if (managerInstances[typeIndex]->isInitialized())
			managerInstances[typeIndex]->deinitialize();
		managerInstances[typeIndex].reset();
	}

	managerInstances.clear();
	managerInstancingOrder.clear();
}

void BaseApplication::mainloop()
{
	resource::ResourceManager &rm = getManager<resource::ResourceManager>();
	debugFont = rm.loadResource<resource::FontResource>("_application_debug_font", "selawk.ttf", true);
	TS_ASSERT(debugFont != nullptr && debugFont->isLoaded() && "Loading debug font failed.");

	TimeSpan deltaAccumulator;

	SizeType frameCounter = 0;
	Clock framerateClock;

	Clock deltaClock;
	Clock loopTimer;

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

		TimeSpan deltaTime = deltaClock.restart();

		for (InstancedManagersList::iterator it = managerInstances.begin();
		     it != managerInstances.end(); ++it)
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

		TimeSpan frameSleep = targetFrameTime - loopTimer.getElapsedTime();
		if (frameSleep > TimeSpan::zero)
			Thread::sleep(frameSleep);

		frameCounter++;
		if (framerateClock.getElapsedTime() > TimeSpan::fromMilliseconds(500))
		{
			float elapsedTime = framerateClock.restart().getSecondsAsFloat();
			currentFramerate = (SizeType)(frameCounter / elapsedTime);
			frameCounter = 0;
		}
	}

	if (currentScene != nullptr)
	{
		currentScene->stop();
	}
}

void BaseApplication::handleEvents()
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

void BaseApplication::handleUpdate(const TimeSpan deltaTime)
{
	if (currentScene == nullptr)
		return;

	currentScene->update(deltaTime);
}

void BaseApplication::handleRendering()
{
	if (currentScene == nullptr)
		return;
	
	system::WindowManager &windowManager = getManager<system::WindowManager>();
	if (!windowManager.isOpen())
		return;

	sf::RenderWindow &renderWindow = windowManager.getRenderWindow();
	renderWindow.clear();
	
	// Application custom view step
	windowManager.useApplicationView();
	currentScene->renderApplication(renderWindow, windowManager.getCurrentView());

	// Interface view step
	windowManager.useInterfaceView();
	currentScene->renderInterface(renderWindow, windowManager.getCurrentView());

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
