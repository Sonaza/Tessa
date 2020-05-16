#include "Precompiled.h"
#include "ViewerEventManager.h"

#include "ts/file/FileUtils.h"

TS_DEFINE_MANAGER_TYPE(app::viewer::ViewerEventManager);

TS_PACKAGE2(app, viewer)

ViewerEventManager::ViewerEventManager()
{
	gigaton.registerClass(this);
}

ViewerEventManager::~ViewerEventManager()
{
	gigaton.unregisterClass(this);
}

bool ViewerEventManager::initialize()
{
	TS_ZONE();

	return true;
}

void ViewerEventManager::deinitialize()
{
	TS_ZONE();
}

// void ViewerEventManager::update(const TimeSpan deltaTime)
// {
// 	TS_ZONE();
// }

void ViewerEventManager::pushEvent(const ViewerEvent &event)
{
	eventQueue.push(std::move(event));
}

bool ViewerEventManager::pollEvent(ViewerEvent &outEvent)
{
	if (eventQueue.empty())
		return false;

	outEvent = std::move(eventQueue.front());
	TS_ASSERTF(outEvent.type != ViewerEvent::Unspecified, "Unspecified events are not valid.");

	eventQueue.pop();
	return true;
}

TS_END_PACKAGE2()
