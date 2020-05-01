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

	virtual bool start() override;
	virtual void stop() override;

protected:
	virtual bool createApplicationManagers() override;

	virtual void initializeConfigDefaults(engine::system::ConfigReader &config) override;

	virtual bool initializeScene() override;

	virtual bool createWindow(engine::window::WindowManager &windowManager) override;
	virtual bool loadArchives(resource::archivist::ArchivistFilesystem &fileSystem) override;

	virtual bool customQuitHandler() override;
};

TS_END_PACKAGE1()
