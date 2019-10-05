#pragma once

#include "ts/engine/system/BaseApplication.h"

TS_DECLARE2(engine, window, WindowManager);
TS_DECLARE2(resource, archivist, ArchivistFilesystem);

TS_DECLARE2(app, viewer, BackgroundFileScanner);

TS_PACKAGE1(app)

class Application : public engine::system::BaseApplication
{
public:
	Application(engine::system::Commando &commando);
	virtual ~Application();

	virtual bool start();
	virtual void stop();

protected:
	virtual bool createApplicationManagers();

	virtual void initializeConfigDefaults(engine::system::ConfigReader &config);

	virtual bool initializeScene();

	virtual bool createWindow(engine::window::WindowManager &windowManager);
	virtual bool loadArchives(resource::archivist::ArchivistFilesystem &fileSystem);

	virtual bool customQuitHandler();
};

TS_END_PACKAGE1()
