#pragma once

#include "ts/engine/system/AbstractManagerBase.h"

#include "ts/engine/window/WindowViewManager.h"

TS_DECLARE1(input, InputManager);

TS_PACKAGE2(engine, window)

struct DroppedFile
{
	String filepath;
	math::VC2I point;
};

struct SystemEventCallbackParams;

class WindowManager : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(window::WindowManager);

public:
	WindowManager();
	virtual ~WindowManager();

	virtual bool initialize() override;
	virtual void deinitialize() override;

	virtual void update(const TimeSpan deltaTime)  override;

	void create(const math::VC2U &videomode, const String &windowTitle, const bool resizable, const bool fullscreen);
	void close();

	bool setWindowIcon(const String &filepath);
	void setWindowTitle(const String &title);

	bool pollEvent(sf::Event &eventParam);

	enum FullscreenMode
	{
		FullscreenMode_Windowed,
		FullscreenMode_Borderless,
		FullscreenMode_Fullscreen,
	};

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

	void setAcceptDropfiles(bool enabled);
	void setMinMaxSize(const math::VC2U &minSize, const math::VC2U &maxSize = math::VC2U(50000, 50000));
	math::VC2U getSize() const;

	void useApplicationView();
	void useInterfaceView();

	const WindowView &getApplicationView() const;
	const WindowView &getInterfaceView() const;
	const WindowView &getCurrentView() const;

	sf::RenderWindow &getRenderWindow();

	void setBorderlessFullscreen(const bool enabled);

	static math::VC2U getNativeResolution();
	static std::vector<math::VC2U> getSupportedResolutions(const bool fullscreen, const math::VC2U &minimumSize = math::VC2U::zero);

	lang::Signal<const math::VC2U &> screenSizeChangedSignal;
	lang::Signal<WindowState> windowStateChangedSignal;

	lang::Signal<const std::vector<DroppedFile> &> filesDroppedSignal;

private:
	class SystemEventCallbackWrapper;
	friend class SystemEventCallbackWrapper;
	bool systemEventCallback(SystemEventCallbackParams *params);

	bool windowCreated = false;

	math::VC2U minSize = math::VC2U(800, 600);
	math::VC2U maxSize = math::VC2U(50000, 50000);

	struct WindowSettings
	{
		math::VC2U size;
		FullscreenMode fullscreenMode = FullscreenMode_Windowed;
		WindowState windowState = WindowState_Maximized;
		uint32_t w32style = 0;
	};
	WindowSettings previousSettings;
	FullscreenMode currentFullscreenMode = FullscreenMode_Windowed;

	ScopedPointer<sf::RenderWindow> renderWindow;
	WindowViewManager::ViewType currentViewType;

	input::InputManager *inputManager = nullptr;
	window::WindowViewManager *windowViewManager = nullptr;

};

TS_END_PACKAGE2()
