#include "Precompiled.h"
#include "Application.h"

#include "ts/lang/common/Debugging.h"

#include "ts/engine/window/WindowManager.h"
#include "ts/resource/archivist/ArchivistFilesystem.h"


#include "ts/resource/ResourceManager.h"
#include "ts/file/FileUtils.h"

#include "ts/ivie/AppConfig.h"
#include "ts/ivie/scenes/ImageViewerScene.h"
#include "ts/ivie/viewer/SupportedFormats.h"

#include "ts/ivie/viewer/ViewerManager.h"
#include "ts/ivie/viewer/ViewerEventManager.h"
#include "ts/ivie/image/FreeImageStaticInitializer.h"

TS_PACKAGE1(app)

Application::Application(engine::system::Commando &commando)
	: engine::system::BaseApplication(commando)
{
	image::FreeImageStaticInitializer::staticInitialize();

	if (debugging::isDebuggerPresent())
	{
		resource::ResourceManager::setResourceRootDirectory(file::getWorkingDirectory());
	}
	else
	{
		resource::ResourceManager::setResourceRootDirectory(file::getExecutableDirectory());
	}
}

Application::~Application()
{
}

bool Application::start()
{
	viewer::ViewerManager &viewerManager = getManager<viewer::ViewerManager>();

	String pathParameter;
	getCommando().getNthParameter(0, pathParameter);

	if (pathParameter.isEmpty())
		pathParameter = file::joinPaths(file::getWorkingDirectory(), "img");

	bool hasRecursiveFlag = getCommando().hasFlag("r") || getCommando().hasFlag("recursive");
	viewerManager.setRecursiveScan(hasRecursiveFlag, false);

	viewerManager.setViewerPath(pathParameter);

	return true;
}

void Application::stop()
{
	
}

bool Application::createApplicationManagers()
{
	createManagerInstance<viewer::ViewerManager>();
	createManagerInstance<viewer::ViewerEventManager>();

	return true;
}

void Application::initializeConfigDefaults(engine::system::ConfigReader &config)
{
	config.setString("General.LogFile", "output.log");

	config.setBoolean("Display.Fullscreen", false);
	config.setBoolean("Display.Maximized", true);
	config.setBoolean("Display.VSync", false);

	config.setUint32("Display.ScreenWidth", APP_DEFAULT_SCREEN_WIDTH);
	config.setUint32("Display.ScreenHeight", APP_DEFAULT_SCREEN_HEIGHT);
}

bool Application::initializeScene()
{
	scenes::ImageViewerScene::DisplayMode displayMode = scenes::ImageViewerScene::DisplayMode::Normal;
	String modeFlag;
	if (getCommando().getFlagParameter("mode", modeFlag))
	{
		if (modeFlag == "normal" || modeFlag == "1")
		{
			displayMode = scenes::ImageViewerScene::DisplayMode::Normal;
		}
		else if (modeFlag == "manga" || modeFlag == "2")
		{
			displayMode = scenes::ImageViewerScene::DisplayMode::Manga;
		}
		else if (modeFlag == "noreset" || modeFlag == "3")
		{
			displayMode = scenes::ImageViewerScene::DisplayMode::NoReset;
		}
	}
	else if (getCommando().hasFlag("normal-mode"))
	{
		displayMode = scenes::ImageViewerScene::DisplayMode::Normal;
	}
	else if (getCommando().hasFlag("manga-mode"))
	{
		displayMode = scenes::ImageViewerScene::DisplayMode::Manga;
	}
	else if (getCommando().hasFlag("noreset-mode"))
	{
		displayMode = scenes::ImageViewerScene::DisplayMode::NoReset;
	}

	return loadScene<scenes::ImageViewerScene>(displayMode);
}

bool Application::createWindow(engine::window::WindowManager &windowManager)
{
	engine::system::ConfigReader &config = getConfig();

	bool automaticResolution = true;

	bool fullscreen = config.getBoolean("Display.Fullscreen", false);
	bool maximized = config.getBoolean("Display.Maximized", true);
	bool vsyncEnabled = config.getBoolean("Display.VSync", false);

	math::VC2U windowSize(APP_DEFAULT_SCREEN_WIDTH, APP_DEFAULT_SCREEN_HEIGHT);
	if (automaticResolution)
	{
		std::vector<math::VC2U> resolutions = windowManager.getSupportedResolutions(fullscreen, math::VC2U(APP_MINIMUM_SCREEN_WIDTH, APP_MINIMUM_SCREEN_HEIGHT));
		if (!resolutions.empty())
		{
			windowSize = resolutions[0];
		}
		else
		{
			TS_LOG_ERROR("Automatic resolution deduction failed, received zero modes. Falling back to default.");
		}
	}
	else
	{
		windowSize.x = math::max(config.getUint32("Display.ScreenWidth", APP_DEFAULT_SCREEN_WIDTH), APP_MINIMUM_SCREEN_WIDTH);
		windowSize.y = math::max(config.getUint32("Display.ScreenHeight", APP_DEFAULT_SCREEN_HEIGHT), APP_MINIMUM_SCREEN_HEIGHT);
	}

	windowManager.create(windowSize, getApplicationDefaultWindowTitle().toUtf8().c_str(), true, fullscreen);

	if (maximized)
		windowManager.setWindowState(engine::window::WindowManager::WindowState_Maximized);

	windowManager.setWindowIcon(APP_WINDOW_ICON_PATH);
	windowManager.setVSyncEnabled(vsyncEnabled);

	windowManager.setAcceptDropfiles(true);

// 	sf::RenderWindow &renderwindow = windowManager.getRenderWindow();
// 	renderwindow.getSystemHandle()
	return true;
}

bool Application::loadArchives(resource::archivist::ArchivistFilesystem &fileSystem)
{
	fileSystem.addArchive(resource::ResourceManager::getAbsoluteResourcePath("resources.tsp"));
	return true;
}

bool Application::customQuitHandler()
{
	return true;
}

TS_END_PACKAGE1()

