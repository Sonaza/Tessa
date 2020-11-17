#include "Precompiled.h"
#include "ViewerFileManager.h"

#include "ts/file/FileUtils.h"
#include "ts/file/FileTime.h"

TS_DEFINE_MANAGER_TYPE(app::viewer::ViewerFileManager);

TS_PACKAGE2(app, viewer)

ViewerFileManager::ViewerFileManager()
{

}

ViewerFileManager::~ViewerFileManager()
{

}

bool ViewerFileManager::initialize()
{
	return true;
}

void ViewerFileManager::deinitialize()
{

}

bool ViewerFileManager::setFilepath(const String &path)
{
	return true;
}

TS_END_PACKAGE2()
