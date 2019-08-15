#include "Precompiled.h"
#include "ViewerStateManager.h"

#include "ts/ivie/util/NaturalSort.h"
#include "ts/ivie/viewer/FileScanner.h"

TS_DEFINE_MANAGER_TYPE(app::viewer::ViewerStateManager);

TS_PACKAGE2(app, viewer)

ViewerStateManager::ViewerStateManager()
{
	gigaton.registerClass(this);
}

ViewerStateManager::~ViewerStateManager()
{
	gigaton.unregisterClass(this);

}
bool ViewerStateManager::initialize()
{
	FileScanner &fs = getGigaton<FileScanner>();
	
	filelistChangedBind.connect(fs.filelistChangedSignal, &ThisClass::updateFileList, this);
// 	filelistChangedBind = fs.filelistChangedSignal.connect(&ThisClass::updateFileList, this);

	return true;
}

void ViewerStateManager::deinitialize()
{

}

void ViewerStateManager::updateFileList()
{
	viewer::FileScanner &fileScanner = getGigaton<viewer::FileScanner>();
	std::vector<std::wstring> filelist = fileScanner.getFileList();

	TS_PRINTF("updateFileList - Filelist has %u files\n", filelist.size());
}

TS_END_PACKAGE2()


