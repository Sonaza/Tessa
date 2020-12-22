#pragma once

#include "ts/file/FileTime.h"

TS_PACKAGE2(app, viewer)

struct ViewerImageFile
{
	String filepath;
	uint32_t directoryHash;
	String type;
	file::FileTime lastModifiedTime;
};

extern ViewerImageFile getViewerImageFileDataForFile(const String &absolutePath, const String &relativePath);

TS_END_PACKAGE2()
