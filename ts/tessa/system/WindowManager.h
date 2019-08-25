#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

#include "ts/tessa/system/WindowViewManager.h"

TS_PACKAGE1(system)

struct SystemEventCallbackParams;

class WindowManager : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(system::WindowManager);

public:
	WindowManager();
	virtual ~WindowManager();

	virtual bool initialize();
	virtual void deinitialize();

	virtual void update(const TimeSpan deltaTime);

	void create(const math::VC2U &videomode, const std::string &windowTitle, const bool resizable, const bool fullscreen);
	void close();

	bool setWindowIcon(const std::string &filepath);

	bool pollEvent(sf::Event &eventParam);

	enum WindowState
	{
		WindowState_Normal,
		WindowState_Maximized,
		WindowState_Minimized,
	};
	WindowState getWindowState() const;
	void setWindowState(WindowState state);

	void setVSyncEnabled(const bool enabled);

	bool isOpen() const;
	bool isInFocus() const;

	math::VC2U getSize() const;

	void useApplicationView();
	void useInterfaceView();

	const WindowView &getCurrentView() const;

	sf::RenderWindow &getRenderWindow();

	std::vector<math::VC2U> getSupportedResolutions(const bool fullscreen, const math::VC2U &minimumSize = math::VC2U::zero);

	lang::Signal<const math::VC2U &> screenSizeChangedSignal;
	lang::Signal<WindowState> windowStateChangedSignal;

private:
	class SystemEventCallbackWrapper;
	friend class SystemEventCallbackWrapper;
	bool systemEventCallback(SystemEventCallbackParams *params);

	bool windowCreated = false;

	sf::RenderWindow renderWindow;
	WindowViewManager::ViewType currentViewType;

	system::WindowViewManager *windowViewManager = nullptr;
};

TS_END_PACKAGE1()
