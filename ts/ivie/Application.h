#pragma once

#include "ts/tessa/system/BaseApplication.h"

TS_DECLARE1(system, WindowManager);
TS_DECLARE1(file, ArchivistFilesystem);

TS_DECLARE2(app, viewer, BackgroundFileScanner);

TS_PACKAGE1(app)

class Application : public system::BaseApplication
{
public:
	Application(Int32 argc, const wchar_t **argv);
	virtual ~Application();

	virtual bool start();
	virtual void stop();

protected:
	virtual bool createApplicationManagers();

	virtual void initializeConfigDefaults(system::ConfigReader &config);

	virtual bool initializeScene();

	virtual bool createWindow(system::WindowManager &windowManager);
	virtual bool loadArchives(file::ArchivistFilesystem &fileSystem);

private:
	ScopedPointer<viewer::BackgroundFileScanner> BackgroundFileScanner;

};

TS_END_PACKAGE1()