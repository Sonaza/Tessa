#include "Precompiled.h"
#include "ts/ivie/Application.h"

#include "ts/tessa/system/WindowManager.h"
#include "ts/tessa/file/ArchivistFilesystem.h"


#include "ts/tessa/resource/ResourceManager.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/ivie/AppConfig.h"
#include "ts/ivie/scenes/ImageViewerScene.h"
#include "ts/ivie/viewer/SupportedFormats.h"

#include "ts/ivie/viewer/BackgroundFileScanner.h"
#include "ts/ivie/viewer/ViewerStateManager.h"
#include "ts/ivie/viewer/image/FreeImageStaticInitializer.h"
#include "ts/ivie/viewer/image/ImageManager.h"

#include "ts/tessa/string/String.h"

#if TS_PLATFORM == TS_WINDOWS //&& TS_BUILD != TS_FINALRELEASE
#include "ts/tessa/common/IncludeWindows.h"
#else
bool IsDebuggerPresent() { return false; }
#endif

TS_PACKAGE1(app)

Application::Application(int32 argc, const wchar_t **argv)
	: system::BaseApplication(argc, argv)
{
	viewer::FreeImageStaticInitializer::staticInitialize();
}

Application::~Application()
{
}

bool Application::start()
{
	if (IsDebuggerPresent())
	{
		resource::ResourceManager::setResourceRootDirectory(file::getWorkingDirectory());
	}
	else
	{
		resource::ResourceManager::setResourceRootDirectory(file::getExecutableDirectory());
	}

	viewer::ViewerStateManager &vsm = getManager<viewer::ViewerStateManager>();

	std::wstring filepath;
	getCommando().getNthParameter(0, filepath);
	if (!filepath.empty() && file::exists(filepath) && file::isFile(filepath))
	{
		vsm.jumpToImageByFilename(filepath);
	}
	else
	{
		vsm.jumpToImage(0);
	}

	SizeType maxsize = sf::Texture::getMaximumSize();
	TS_PRINTF("Texture max size %u\n", maxsize);

	return true;
}

void Application::stop()
{
	BackgroundFileScanner.reset();
}

bool Application::createApplicationManagers()
{
	std::wstring workingDirectory;
	getCommando().getNthParameter(0, workingDirectory);

	workingDirectory = file::isFile(workingDirectory) ? file::getDirname(workingDirectory) : workingDirectory;
	if (workingDirectory.empty())
	{
		workingDirectory = file::getWorkingDirectory();
		workingDirectory = file::joinPaths(workingDirectory, "img");
	}

	createManagerInstance<viewer::BackgroundFileScanner>(workingDirectory, viewer::SupportedFormats::getSupportedFormatExtensions());
	createManagerInstance<viewer::ViewerStateManager>();
	createManagerInstance<viewer::ImageManager>();

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

// 	sf::RenderWindow &renderwindow = windowManager.getRenderWindow();
// 	renderwindow.getSystemHandle()
	return true;
}

bool Application::loadArchives(file::ArchivistFilesystem &fileSystem)
{

	return true;
}

TS_END_PACKAGE1()

