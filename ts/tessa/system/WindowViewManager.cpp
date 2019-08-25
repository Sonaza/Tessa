#include "Precompiled.h"
#include "ts/tessa/system/WindowViewManager.h"

#include "ts/tessa/system/WindowManager.h"

#include "ts/tessa/resource/ResourceManager.h"

#if TS_PLATFORM == TS_WINDOWS
	#include "ts/tessa/common/IncludeWindows.h"
#endif

TS_DEFINE_MANAGER_TYPE(system::WindowViewManager);

TS_PACKAGE1(system)

WindowViewManager::WindowViewManager()
{
	gigaton.registerClass(this);
}

WindowViewManager::~WindowViewManager()
{
	gigaton.unregisterClass(this);
}

bool WindowViewManager::initialize()
{
	windowManager = gigaton.getGigatonOptional<system::WindowManager>();
	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(false, windowManager);

	screenSizeChangedBind.connect(windowManager->screenSizeChangedSignal, &ThisClass::screenSizeChanged, this);

	return true;
}

void WindowViewManager::deinitialize()
{
	screenSizeChangedBind.disconnect();
}

void WindowViewManager::update(const TimeSpan deltaTime)
{

}

sf::View WindowViewManager::getSFMLView(ViewType type) const
{
	TS_ASSERT(type >= ViewType_Application && type <= ViewType_Interface);
	return static_cast<sf::View>(views[type]);
}

const WindowView &WindowViewManager::getView(ViewType type) const
{
	TS_ASSERT(type >= ViewType_Application && type <= ViewType_Interface);
	return views[type];
}

void WindowViewManager::screenSizeChanged(const math::VC2U &sizeParam)
{
	math::VC2 size = static_cast<math::VC2>(sizeParam);

	views[ViewType_Application].size = size;
// 	views[ViewType_Interface].position = math::VC2::zero;

	views[ViewType_Interface].size = size;
	views[ViewType_Interface].position = size / 2.f;
	views[ViewType_Interface].rotation = 0.f;
	views[ViewType_Interface].scale = 1.f;
}

TS_END_PACKAGE1()

