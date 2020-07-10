#include "Precompiled.h"
#include "ViewerImageFile.h"

#include "ts/file/FileUtils.h"
#include "ts/file/FileTime.h"

TS_PACKAGE2(app, viewer)

extern ViewerImageFile getViewerImageFileDataForFile(const String &absolutePath, const String &relativePath)
{
	ViewerImageFile result;
	result.filepath = relativePath;

	result.directoryHash = math::simpleHash32(file::getDirname(absolutePath));

	result.lastModifiedTime = file::getFileModifiedTime(absolutePath);
	result.type = file::getShellFileType(absolutePath);

	return result;
}

TS_END_PACKAGE2()
