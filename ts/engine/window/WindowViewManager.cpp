#include "Precompiled.h"
#include "WindowViewManager.h"

#include "ts/engine/window/WindowManager.h"

#include "ts/resource/ResourceManager.h"

#if TS_PLATFORM == TS_WINDOWS
#include "ts/lang/common/IncludeWindows.h"
#endif

TS_DEFINE_MANAGER_TYPE(engine::window::WindowViewManager);

TS_PACKAGE2(engine, window)

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
	windowManager = gigaton.getGigatonOptional<window::WindowManager>();
	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(false, windowManager);

	screenSizeChangedBind.connect(
		windowManager->screenSizeChangedSignal,
		lang::SignalPriority_VeryHigh,
		&ThisClass::screenSizeChanged, this);

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

TS_END_PACKAGE2()

