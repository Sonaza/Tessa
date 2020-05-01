#pragma once

#include "ts/engine/system/AbstractManagerBase.h"

#include "ts/engine/window/WindowView.h"

TS_DECLARE2(engine, window, WindowManager);

TS_PACKAGE2(engine, window)

class WindowViewManager : public engine::system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(engine::window::WindowViewManager);

public:
	WindowViewManager();
	virtual ~WindowViewManager();

	virtual bool initialize() override;
	virtual void deinitialize() override;

	virtual void update(const TimeSpan deltaTime) override;

	enum ViewType
	{
		ViewType_Application,
		ViewType_Interface,
	};
	sf::View getSFMLView(ViewType type) const;
	const WindowView &getView(ViewType type) const;

private:
	void screenSizeChanged(const math::VC2U &size);
	lang::SignalBind screenSizeChangedBind;

	WindowView views[2];

	window::WindowManager *windowManager = nullptr;
};

TS_END_PACKAGE2()
