#pragma once

#include "ts/ivie/viewer/ViewerEvent.h"
#include <queue>

TS_PACKAGE2(app, viewer)

class ViewerEventManager : public engine::system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(app::viewer::ViewerEventManager);

public:
	ViewerEventManager();
	~ViewerEventManager();

	virtual bool initialize() override;
	virtual void deinitialize() override;

// 	virtual void update(const TimeSpan deltaTime) override;

	void pushEvent(const ViewerEvent &event);
	bool pollEvent(ViewerEvent &outEvent);

private:
	std::queue<ViewerEvent> eventQueue;
};

TS_END_PACKAGE2()
