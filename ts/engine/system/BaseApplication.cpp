#include "Precompiled.h"
#include "BaseApplication.h"

#include "ts/lang/common/Debugging.h"
#include "ts/resource/archivist/ArchivistFilesystem.h"
#include "ts/file/FileUtils.h"
#include "ts/input/InputManager.h"
#include "ts/profiling/ZoneProfiler.h"
#include "ts/resource/FontResource.h"
#include "ts/resource/ResourceManager.h"
#include "ts/engine/Gigaton.h"
#include "ts/engine/window/WindowManager.h"
#include "ts/engine/window/WindowViewManager.h"
#include "ts/thread/Thread.h"
#include "ts/thread/ThreadScheduler.h"
#include "ts/thread/ThreadUtils.h"

TS_PACKAGE2(engine, system)

BaseApplication::BaseApplication(system::Commando &commando)
	: m_gigatonInstance(TS_GET_GIGATON())
	, m_commando(std::move(commando))
{
	thread::Thread::setMainThread(thread::Thread::getCurrentThread());
	m_gigatonInstance.registerClass(this);
}

BaseApplication::~BaseApplication()
{
	m_gigatonInstance.unregisterClass(this);
}

int32 BaseApplication::launch()
{
	printf("HELLO WORLD\n");
	
	initializeConfigDefaults(m_config);

	String rootPath;
	if (debugging::isDebuggerPresent())
	{
		rootPath = file::getWorkingDirectory();
	}
	else
	{
		rootPath = file::getExecutableDirectory();
	}
	
	TS_PRINTF("rootpath %s\n", rootPath);

	const String configFilepath = file::joinPaths(rootPath, getApplicationConfigFile());
	TS_PRINTF("configFilepath %s\n", configFilepath);

	if (!m_config.open(configFilepath))
	{
		if (!file::exists(configFilepath))
		{
			// Config doesn't exist, create a default.
			m_config.save(configFilepath);
			TS_WLOG_WARNING("Unable to open config file. File: %s. Created a new file with application defaults.", configFilepath);
		}
		else
		{
			TS_WLOG_WARNING("Unable to open config file. File: %s. File exists but likely has syntax errors.", configFilepath);
		}
	}

// 	const String logFilepath = file::joinPaths(
// 		file::getExecutableDirectory(), m_config.getString("General.LogFile", TS_DEFAULT_LOG_FILE_NAME));
// 	common::Log::setLogFile(logFilepath);

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

bool BaseApplication::quitImpl(bool force)
{
	bool quitHandlerResult = customQuitHandler();
	if (force == false && quitHandlerResult == false) // Ignores quit handler if forced
		return false;

#if TS_BUILD != TS_DEBUG
	fastExit();
#else
	m_applicationRunning = false;
	window::WindowManager &windowManager = getManager<window::WindowManager>();
	windowManager.close();
	return true;
#endif
}

void BaseApplication::fastExit()
{
// 	std::quick_exit(123);
	_Exit(123);
}

bool BaseApplication::initialize()
{
	TS_ZONE();

	if (!createSystemManagers())
		return false;

	if (!createApplicationManagers())
		return false;

	TS_ASSERT(m_managerInstances.size() == m_managerInstancingOrder.size());
	for (auto it = m_managerInstancingOrder.begin(); it != m_managerInstancingOrder.end(); ++it)
	{
		const std::type_index &typeIndex = *it;
		if (m_managerInstances[typeIndex]->initialize())
		{
			m_managerInstances[typeIndex]->initialized = true;
		}
		else
		{
			TS_LOG_ERROR("Initializing a manager instance failed. Manager type: %s", m_managerInstances[typeIndex]->getTypeName());
			return false;
		}
	}

	resource::archivist::ArchivistFilesystem &afs = getManager<resource::archivist::ArchivistFilesystem>();
	if (!loadArchives(afs))
		return false;

	window::WindowManager &wm = getManager<window::WindowManager>();
	if (!createWindow(wm))
	{
		TS_LOG_ERROR("Window creation failed.");
		return false;
	}

	return true;
}

void BaseApplication::deinitialize()
{
	if (m_currentScene != nullptr)
		m_currentScene->internalStop();

	m_pendingScene.reset();
	m_currentScene.reset();

	destroyManagerInstances();
}

void BaseApplication::setFramerateLimit(SizeType framerateLimit)
{
	if (framerateLimit == 0)
	{
		m_targetFrameTime = TimeSpan::fromMilliseconds(16);
		return;
	}

	m_targetFrameTime = TimeSpan::fromMilliseconds(1000 / framerateLimit);
}

SizeType BaseApplication::getCurrentFramerate() const
{
	return m_currentFramerate;
}

const Commando &BaseApplication::getCommando() const
{
	return m_commando;
}

ConfigReader &BaseApplication::getConfig()
{
	return m_config;
}

const ConfigReader &BaseApplication::getConfig() const
{
	return m_config;
}

sf::Font &BaseApplication::getDebugFont()
{
	return *m_debugFont->getResource();
}

bool BaseApplication::requestQuit()
{
	return quitImpl();
}

void BaseApplication::forceQuit()
{
	quitImpl(true);
}

bool BaseApplication::createSystemManagers()
{
	TS_ZONE();

	if (!createManagerInstance<thread::ThreadScheduler>())
		return false;

	if (!createManagerInstance<resource::archivist::ArchivistFilesystem>())
		return false;
	
	if (!createManagerInstance<resource::ResourceManager>())
		return false;

	if (!createManagerInstance<window::WindowManager>())
		return false;

	if (!createManagerInstance<window::WindowViewManager>())
		return false;

	if (!createManagerInstance<input::InputManager>())
		return false;

	return true;
}

void BaseApplication::destroyManagerInstances()
{
	TS_ASSERT(m_managerInstances.size() == m_managerInstancingOrder.size());
	for (auto it = m_managerInstancingOrder.rbegin(); it != m_managerInstancingOrder.rend(); ++it)
	{
		const std::type_index &typeIndex = *it;
		if (m_managerInstances[typeIndex]->isInitialized())
			m_managerInstances[typeIndex]->deinitialize();
		m_managerInstances[typeIndex].reset();
	}

	m_managerInstances.clear();
	m_managerInstancingOrder.clear();
}

void BaseApplication::mainloop()
{
// 	TS_ZONE_NAMED_VARIABLE(baseMainLoop, "Base Main Loop");

	resource::ResourceManager &rm = getManager<resource::ResourceManager>();
	m_debugFont = rm.loadResource<resource::FontResource>("_application_debug_font", "selawk.ttf", true);
	TS_ASSERT(m_debugFont != nullptr && m_debugFont->isLoaded() && "Loading debug font failed.");

	TimeSpan deltaAccumulator;

	SizeType frameCounter = 0;
	SteadyTimer framerateClock;

	SteadyTimer deltaClock;
	SteadyTimer frameTimer;

	while (m_applicationRunning)
	{
		TS_ZONE_NAMED("Main Loop");

		frameTimer.restart();

		if (m_pendingScene != nullptr)
		{
			bool successfulStart = m_pendingScene->internalStart();
			if (!successfulStart)
			{
				TS_LOG_ERROR("Pending scene start failed. Scene will not be loaded.");
				m_pendingScene.reset();
			}

			if (m_currentScene != nullptr)
			{
				m_currentScene->internalStop();
				m_currentScene.reset();
			}

			m_currentScene = std::move(m_pendingScene);
			m_currentScene->loadResources(getManager<resource::ResourceManager>());
		}

		TimeSpan deltaTime = deltaClock.restart();

		for (auto it = m_managerInstancingOrder.begin(); it != m_managerInstancingOrder.end(); ++it)
		{
			const std::type_index &typeIndex = *it;
			m_managerInstances[typeIndex]->update(deltaTime);
		}

		handleEvents();

		if (!m_applicationRunning)
			break;

		if (m_currentScene != nullptr)
		{
			TS_ZONE_NAMED("Main Loop Updates");

			m_currentScene->handleInput(getManager<input::InputManager>());

			deltaAccumulator += deltaTime;
			while (deltaAccumulator >= m_fixedDeltaTime)
			{
				m_currentScene->update(m_fixedDeltaTime);
				deltaAccumulator -= m_fixedDeltaTime;
			}

			m_currentScene->updateFrequent(deltaTime);
		}

		handleRendering();

		{
			TS_ZONE_NAMED("Main Loop Sleep");
			TimeSpan elapsedFrameTime = frameTimer.getElapsedTime();
			TimeSpan frameSleep = m_targetFrameTime - elapsedFrameTime;
			frameSleep = math::max(2_ms, frameSleep);
//	 		TS_PRINTF("Frame Time %lldms / sleep %lldms\n", elapsedFrameTime.getMilliseconds(), frameSleep.getMilliseconds());
			Thread::sleep(frameSleep);
		}

		frameCounter++;
		if (framerateClock.getElapsedTime() > 500_ms)
		{
			float elapsedTime = framerateClock.restart().getSecondsAsFloat();
			m_currentFramerate = (SizeType)(frameCounter / elapsedTime);
			frameCounter = 0;
		}

// 		baseMainLoop.commit();
	}

	if (m_currentScene != nullptr)
	{
		m_currentScene->stop();
	}
}

void BaseApplication::handleEvents()
{
	TS_ZONE();

	window::WindowManager &windowManager = getManager<window::WindowManager>();
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
		if (m_currentScene != nullptr)
		{
			// If event is handled by scene the rest can be skipped
			if (m_currentScene->handleEvent(event))
				continue;
		}

		switch (event.type)
		{
			case sf::Event::Closed:
			{
				if (quitImpl())
					return;
			}
			break;

			case sf::Event::KeyPressed:
			{
				switch (event.key.code)
				{
					case sf::Keyboard::Escape:
					{
						if (quitImpl())
							return;
					}
					break;

					case sf::Keyboard::F9:
					{
						m_showFramesPerSecond = !m_showFramesPerSecond;
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

					default: break;
				}
			}
			break;
			
			default: break;
		}
	}
}

void BaseApplication::handleRendering()
{
	TS_ZONE();

	TS_ZONE_NAMED_VARIABLE(firstHalfZone, "First Half Zone");

	if (m_currentScene == nullptr)
		return;
	
	window::WindowManager &windowManager = getManager<window::WindowManager>();
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

	TS_ZONE_VARIABLE_FINISH(firstHalfZone);
	
	TS_ZONE_NAMED_VARIABLE(secondHalfZone, "Second Half Zone");

	// Application custom view step
	windowManager.useApplicationView();
	m_currentScene->renderApplication(renderWindow, windowManager.getCurrentView());

	// Interface view step
	windowManager.useInterfaceView();
	m_currentScene->renderInterface(renderWindow, windowManager.getCurrentView());

	TS_ZONE_VARIABLE_FINISH(secondHalfZone);

#if TS_PROFILER_ENABLED == TS_TRUE
	if (profiling::ZoneProfiler::isVisible())
	{
		profiling::ZoneProfiler::render(renderWindow, windowManager.getCurrentView());
	}
#endif

	if (m_showFramesPerSecond && m_debugFont != nullptr && m_debugFont->isLoaded())
	{
		TS_ZONE_NAMED("FPS Draw");

		sf::Text fps(
			TS_FMT("FPS %u", getCurrentFramerate()), *m_debugFont->getResource(), 30
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

TS_END_PACKAGE2()
