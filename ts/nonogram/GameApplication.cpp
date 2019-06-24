#include "Precompiled.h"
#include "ts/nonogram/GameApplication.h"

#include "ts/tessa/system/WindowManager.h"
#include "ts/tessa/file/ArchivistFilesystem.h"

#include "ts/nonogram/scenes/NonogramScene.h"
#include "ts/nonogram/GameConfig.h"

TS_PACKAGE1(game)

GameApplication::GameApplication(Int32 argc, const char **argv)
	: Application(argc, argv)
{
}

GameApplication::~GameApplication()
{
}

bool GameApplication::sceneInitialize()
{
	return loadScene<scenes::NonogramScene>();
}

bool GameApplication::createWindow(system::WindowManager &windowManager)
{
	const system::ConfigReader &config = getConfig();

	bool automaticResolution = true;

	bool fullscreen = config.getBoolean("Display.Fullscreen", false);
	bool vsyncEnabled = config.getBoolean("Display.VSync", false);

	math::VC2U windowSize(GAME_DEFAULT_SCREEN_WIDTH, GAME_DEFAULT_SCREEN_HEIGHT);
	if (automaticResolution)
	{
		std::vector<math::VC2U> resolutions = windowManager.getSupportedResolutions(fullscreen, math::VC2U(GAME_MINIMUM_SCREEN_WIDTH, GAME_MINIMUM_SCREEN_HEIGHT));
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
		windowSize.x = math::max(config.getUint32("Display.ScreenWidth", GAME_DEFAULT_SCREEN_WIDTH), GAME_MINIMUM_SCREEN_WIDTH);
		windowSize.y = math::max(config.getUint32("Display.ScreenHeight", GAME_DEFAULT_SCREEN_HEIGHT), GAME_MINIMUM_SCREEN_HEIGHT);
	}

	windowManager.create(windowSize, GAME_WINDOW_TITLE, false, fullscreen);

	windowManager.setWindowIcon(GAME_WINDOW_ICON_PATH);
	windowManager.setVSyncEnabled(vsyncEnabled);

// 	sf::RenderWindow &renderwindow = windowManager.getRenderWindow();
// 	renderwindow.getSystemHandle()
	return true;
}

bool GameApplication::loadArchives(file::ArchivistFilesystem &fileSystem)
{
	fileSystem.addArchive("afs/test.tspack");
	fileSystem.addArchive("afs/phancy.tspack");
	fileSystem.addArchive("afs/music.tspack");
	return true;
}

TS_END_PACKAGE1()

