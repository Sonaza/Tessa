#pragma once

TS_PACKAGE2(app, viewer)

struct ViewerEvent
{
	ViewerEvent();
	~ViewerEvent();

	ViewerEvent(const ViewerEvent &);
	ViewerEvent &operator=(const ViewerEvent &);

	ViewerEvent(ViewerEvent &&);
	ViewerEvent &operator=(ViewerEvent &&);

	enum Type
	{
		Unspecified,
		ImageDeleted,
		ModeChanged,
	};

	Type type = Unspecified;
	
	// Generic data variables that types can use
	String dataString;
	bool dataBool = false;
};

TS_END_PACKAGE2()
