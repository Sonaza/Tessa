#include "Precompiled.h"
#include "BaseApplication.h"

#include "ts/tessa/common/Debugging.h"

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

#include "ts/tessa/profiling/ZoneProfiler.h"

TS_PACKAGE1(system)

BaseApplication::BaseApplication(int32 argc, const char **argv)
	: gigaton(TS_GET_GIGATON())
	, _commando(argc, argv)
{
	thread::Thread::setMainThread(thread::Thread::getCurrentThread());

	gigaton.registerClass(this);
}

BaseApplication::BaseApplication(int32 argc, const wchar_t **argv)
	: gigaton(TS_GET_GIGATON())
	, _commando(argc, argv)
{
	gigaton.registerClass(this);
}

BaseApplication::~BaseApplication()
{
	gigaton.unregisterClass(this);
}

int32 BaseApplication::launch()
{
	initializeConfigDefaults(_config);

	String rootPath;
	if (debugging::isDebuggerPresent())
	{
		rootPath = file::getWorkingDirectory();
	}
	else
	{
		rootPath = file::getExecutableDirectory();
	}

	const String configFilepath = file::joinPaths(rootPath, TS_CONFIG_FILE_NAME);

	if (!_config.open(configFilepath))
	{
		if (!file::exists(configFilepath))
		{
			// Config doesn't exist, create a default.
			_config.save(configFilepath);
			TS_WLOG_WARNING("Unable to open config file. File: %s. Created a new file with application defaults.", configFilepath);
		}
		else
		{
			TS_WLOG_WARNING("Unable to open config file. File: %s. File exists but likely has syntax errors.", configFilepath);
		}
	}

	const String logFilepath = file::joinPaths(
		file::getExecutableDirectory(), _config.getString("General.LogFile", TS_DEFAULT_LOG_FILE_NAME));
	common::Log::setLogFile(logFilepath);

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
	
	if (!initializeScene())
	{
		TS_LOG_ERROR("Scene initialize failed!\n");
		return -1;
	}

	mainloop();

	stop();

	deinitialize();

	return 0;
}

bool BaseApplication::initialize()
{
	TS_ZONE();

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
	TS_ZONE();

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
// 	TS_ZONE_NAMED_VARIABLE(baseMainLoop, "Base Main Loop");

	resource::ResourceManager &rm = getManager<resource::ResourceManager>();
	debugFont = rm.loadResource<resource::FontResource>("_application_debug_font", "selawk.ttf", true);
	TS_ASSERT(debugFont != nullptr && debugFont->isLoaded() && "Loading debug font failed.");

	TimeSpan deltaAccumulator;

	SizeType frameCounter = 0;
	Clock framerateClock;

	Clock deltaClock;
	Clock frameTimer;

	while (applicationRunning)
	{
		TS_ZONE_NAMED("Main Loop");

		frameTimer.restart();

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

		for (auto it = managerInstancingOrder.begin(); it != managerInstancingOrder.end(); ++it)
		{
			const std::type_index &typeIndex = *it;
			managerInstances[typeIndex]->update(deltaTime);
		}

		handleEvents();

		if (!applicationRunning)
			break;

		SizeType updates = 0;

		{
			TS_ZONE_NAMED("Main Loop Updates");
			deltaAccumulator += deltaTime;
			while (deltaAccumulator >= fixedDeltaTime)
			{
				handleUpdate(fixedDeltaTime);
				deltaAccumulator -= fixedDeltaTime;
				updates++;
			}
// 			if (updates > 1)
// 				TS_PRINTF("Update lagged, catched up by updating %u times\n", updates);
		}

		handleRendering();

		{
			TS_ZONE_NAMED("Main Loop Sleep");
			TimeSpan elapsedFrameTime = frameTimer.getElapsedTime();
			TimeSpan frameSleep = targetFrameTime - elapsedFrameTime;
			frameSleep = math::max(2_ms, frameSleep);
//	 		TS_PRINTF("Frame Time %lldms / sleep %lldms\n", elapsedFrameTime.getMilliseconds(), frameSleep.getMilliseconds());
			Thread::sleep(frameSleep);
		}

		frameCounter++;
		if (framerateClock.getElapsedTime() > 500_ms)
		{
			float elapsedTime = framerateClock.restart().getSecondsAsFloat();
			currentFramerate = (SizeType)(frameCounter / elapsedTime);
			frameCounter = 0;
		}

// 		baseMainLoop.commit();
	}

	if (currentScene != nullptr)
	{
		currentScene->stop();
	}
}

void BaseApplication::handleEvents()
{
	TS_ZONE();

	system::WindowManager &windowManager = getManager<system::WindowManager>();
	if (!windowManager.isOpen())
		return;

	sf::Event event;
	while (windowManager.pollEvent(event))
	{
#if TS_PROFILER_ENABLED == TS_TRUE
		if (profiling::ZoneProfiler::isVisible())
		{
			if (profiling::ZoneProfiler::handleEvent(event))
				continue;
		}
		else
#endif
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
				switch (event.key.code)
				{
					case sf::Keyboard::Escape:
					{
						applicationRunning = false;

						system::WindowManager &windowManager = getManager<system::WindowManager>();
						windowManager.close();
						return;
					}
					break;

					case sf::Keyboard::F9:
					{
						showFPS = !showFPS;
					}
					break;

					case sf::Keyboard::F10:
					{
						bool enabled = !profiling::ZoneProfiler::isEnabled();
						profiling::ZoneProfiler::setEnabled(enabled);

						TS_PRINTF("Zone profiler is now %s\n", enabled ? "enabled" : "disabled");
					}
					break;

					case sf::Keyboard::F11:
					{
						profiling::ZoneProfiler::save("profile.xml");
					}
					break;

					case sf::Keyboard::Multiply:
					{
						profiling::ZoneProfiler::toggleVisibility();
					}
					break;


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
	TS_ZONE();

	TS_ZONE_NAMED_VARIABLE(firstHalfZone, "First Half Zone");

	if (currentScene == nullptr)
		return;
	
	system::WindowManager &windowManager = getManager<system::WindowManager>();
	if (!windowManager.isOpen())
		return;

	sf::RenderWindow &renderWindow = windowManager.getRenderWindow();

// 	{
// 		TS_ZONE_NAMED("renderWindow.setActive");
// 		renderWindow.setActive(true);
// 	}

	{
		TS_ZONE_NAMED("renderWindow.clear");
		renderWindow.clear();
	}

	firstHalfZone.commit();
	
	TS_ZONE_NAMED_VARIABLE(secondHalfZone, "Second Half Zone");

	// Application custom view step
	windowManager.useApplicationView();
	currentScene->renderApplication(renderWindow, windowManager.getCurrentView());

	// Interface view step
	windowManager.useInterfaceView();
	currentScene->renderInterface(renderWindow, windowManager.getCurrentView());

	secondHalfZone.commit();

#if TS_PROFILER_ENABLED == TS_TRUE
	if (profiling::ZoneProfiler::isVisible())
	{
		profiling::ZoneProfiler::render(renderWindow, windowManager.getCurrentView());
	}
#endif

	if (showFPS && debugFont != nullptr && debugFont->isLoaded())
	{
		TS_ZONE_NAMED("FPS Draw");

		sf::Text fps(
			TS_FMT("FPS %u", getCurrentFramerate()), *debugFont->getResource(), 30
		);
		fps.setScale(0.75f, 0.75f);
// 		fps.setCharacterSize(30);
// 		fps.setFillColor(sf::Color::White);
// 		fps.setOutlineColor(sf::Color::Black);
// 		fps.setOutlineThickness(1.f);

		renderWindow.draw(fps);
	}

	{
		TS_ZONE_NAMED("renderWindow.display");
		renderWindow.display();
	}
}

TS_END_PACKAGE1()
