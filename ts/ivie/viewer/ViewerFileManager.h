#pragma once

#include "ts/file/FileTime.h"

TS_PACKAGE2(app, viewer)

class ViewerFileManager : public engine::system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(app::viewer::ViewerFileManager);

public:
	ViewerFileManager();
	~ViewerFileManager();

	virtual bool initialize() override;
	virtual void deinitialize() override;

	bool setFilepath(const String &path);

private:
	String currentFilepath;

};

TS_END_PACKAGE2()
