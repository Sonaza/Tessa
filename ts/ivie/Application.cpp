#include "Precompiled.h"
#include "Application.h"

#include "ts/tessa/common/Debugging.h"

#include "ts/tessa/system/WindowManager.h"
#include "ts/tessa/file/ArchivistFilesystem.h"


#include "ts/tessa/resource/ResourceManager.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/ivie/AppConfig.h"
#include "ts/ivie/scenes/ImageViewerScene.h"
#include "ts/ivie/viewer/SupportedFormats.h"

#include "ts/ivie/viewer/ViewerManager.h"
#include "ts/ivie/image/FreeImageStaticInitializer.h"

TS_PACKAGE1(app)

Application::Application(int32 argc, const wchar_t **argv)
	: system::BaseApplication(argc, argv)
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

	bool hasRecursiveFlag = getCommando().hasFlag("r") || getCommando().hasFlag("-recursive");
	viewerManager.setRecursiveScan(hasRecursiveFlag, false);

	viewerManager.setFilepath(pathParameter);

	return true;
}

void Application::stop()
{
	
}

bool Application::createApplicationManagers()
{
	createManagerInstance<viewer::ViewerManager>();

	return true;
}

void Application::initializeConfigDefaults(system::ConfigReader &config)
{
	config.setString("General.LogFile", "output.log");

	config.setBoolean("Display.Fullscreen", false);
	config.setBoolean("Display.Maximized", false);
	config.setBoolean("Display.VSync", false);

	config.setUint32("Display.ScreenWidth", APP_DEFAULT_SCREEN_WIDTH);
	config.setUint32("Display.ScreenHeight", APP_DEFAULT_SCREEN_HEIGHT);
}

bool Application::initializeScene()
{
	return loadScene<scenes::ImageViewerScene>();
}

bool Application::createWindow(system::WindowManager &windowManager)
{
	system::ConfigReader &config = getConfig();

	bool automaticResolution = true;

	bool fullscreen = config.getBoolean("Display.Fullscreen", false);
	bool maximized = config.getBoolean("Display.Maximized", false);
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

	windowManager.create(windowSize, APP_WINDOW_TITLE, true, fullscreen);

	if (maximized)
		windowManager.setWindowState(system::WindowManager::WindowState_Maximized);

	windowManager.setWindowIcon(APP_WINDOW_ICON_PATH);
	windowManager.setVSyncEnabled(vsyncEnabled);

	windowManager.setAcceptDropfiles(true);

// 	sf::RenderWindow &renderwindow = windowManager.getRenderWindow();
// 	renderwindow.getSystemHandle()
	return true;
}

bool Application::loadArchives(file::ArchivistFilesystem &fileSystem)
{
	fileSystem.addArchive(resource::ResourceManager::getAbsoluteResourcePath("resources.tsp"));
	return true;
}

TS_END_PACKAGE1()

