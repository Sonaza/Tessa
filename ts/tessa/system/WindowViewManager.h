#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

TS_DECLARE1(system, WindowManager);

TS_PACKAGE1(system)

struct WindowView
{
	math::VC2 position;    // The center position
	math::VC2 size;        // The size in pixels
	float rotation = 0.f;  // The rotation in degrees
	float scale = 1.f;     // Scales the view size in a way where >1.0 results in thigs looking smaller 

	explicit operator sf::View() const;

	sf::Transform getTransform() const;

	math::VC2 convertToViewCoordinate(const math::VC2 &coordinate) const;
	math::VC2 convertFromViewCoordinate(const math::VC2 &coordinate) const;
};

class WindowViewManager : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(system::WindowViewManager);

public:
	WindowViewManager();
	virtual ~WindowViewManager();

	virtual bool initialize();
	virtual void deinitialize();

	virtual void update(const TimeSpan deltaTime);

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

	system::WindowManager *windowManager = nullptr;
};

TS_END_PACKAGE1()
