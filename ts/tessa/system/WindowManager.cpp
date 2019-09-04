#include "Precompiled.h"
#include "ts/tessa/system/WindowManager.h"

#include "ts/tessa/resource/ResourceManager.h"
#include "ts/tessa/resource/ImageResource.h"

#if TS_PLATFORM == TS_WINDOWS

#include "ts/tessa/common/IncludeWindows.h"
#include "shellapi.h"

#endif

TS_DEFINE_MANAGER_TYPE(system::WindowManager);

TS_PACKAGE1(system)

#if TS_PLATFORM == TS_WINDOWS

struct SystemEventCallbackParams
{
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
};

class WindowManager::SystemEventCallbackWrapper
{
public:
	static WindowManager *windowManager;
	static bool windowsSystemEventCallback(UINT message, WPARAM wParam, LPARAM lParam)
	{
		TS_ASSERT(SystemEventCallbackWrapper::windowManager != nullptr);
// 		WindowManager *wm = TS_GET_GIGATON().getGigatonOptional<WindowManager>();
// 		if (wm == nullptr)
// 			return false;

		SystemEventCallbackParams params = { message, wParam, lParam };
		return SystemEventCallbackWrapper::windowManager->systemEventCallback(&params);
	}
};

WindowManager *WindowManager::SystemEventCallbackWrapper::windowManager = nullptr;

#endif

WindowManager::WindowManager()
{
	gigaton.registerClass(this);
}

WindowManager::~WindowManager()
{
	gigaton.unregisterClass(this);
}

bool WindowManager::initialize()
{
	windowViewManager = gigaton.getGigatonOptional<system::WindowViewManager>();
	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(false, windowViewManager);

	return true;
}

void WindowManager::deinitialize()
{
	close();
}

void WindowManager::update(const TimeSpan deltaTime)
{

}

void WindowManager::create(const math::VC2U &videomode, const String &windowTitle, const bool resizable, const bool fullscreen)
{
	uint32_t style = sf::Style::Titlebar | sf::Style::Close;
	if (fullscreen)
	{
		style = sf::Style::Fullscreen;
	}
	else if (resizable)
	{
		style |= sf::Style::Resize;
	}

	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;
	settings.majorVersion = 3;
	settings.minorVersion = 0;

	renderWindow.reset(new sf::RenderWindow(
		sf::VideoMode(videomode.x, videomode.y),
		windowTitle, style, settings
	));

	if (renderWindow != nullptr)
	{
		screenSizeChangedSignal(videomode);

		renderWindow->clear();
		renderWindow->display();

#if TS_PLATFORM == TS_WINDOWS
		SystemEventCallbackWrapper::windowManager = this;
		renderWindow->setCustomSystemEventCallback((void*)&SystemEventCallbackWrapper::windowsSystemEventCallback);
#endif
	}
}

void WindowManager::close()
{
	renderWindow->close();
	renderWindow.reset();
}

void WindowManager::setVSyncEnabled(const bool enabled)
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");
	renderWindow->setVerticalSyncEnabled(enabled);
}

void WindowManager::setWindowState(WindowState state)
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");

#if TS_PLATFORM == TS_WINDOWS
	int32 flags = SW_NORMAL;
	switch (state)
	{
		case WindowState_Normal:    flags = SW_RESTORE;  break;
		case WindowState_Maximized: flags = SW_MAXIMIZE; break;
		case WindowState_Minimized: flags = SW_MINIMIZE; break;
		default: TS_ASSERT(!"Unhandled window state"); break;
	}
	ShowWindow(renderWindow->getSystemHandle(), flags);
#else
	TS_ASSERT(!"Not implemented on this platform.");
#endif

	renderWindow->clear();
	renderWindow->display();
}

WindowManager::WindowState WindowManager::getWindowState() const
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");

#if TS_PLATFORM == TS_WINDOWS
	WINDOWPLACEMENT placement;
	placement.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(renderWindow->getSystemHandle(), &placement))
	{
		switch (placement.showCmd)
		{
			case SW_SHOWNORMAL:     return WindowState_Normal;
			case SW_SHOWMINIMIZED:  return WindowState_Maximized;
			case SW_SHOWMAXIMIZED:  return WindowState_Minimized;
			default: /* bop */ break;
		}
	}

#else
	TS_ASSERT(!"Not implemented on this platform.");
#endif

	// Unknown state, just return normal
	return WindowState_Normal;
}

bool WindowManager::pollEvent(sf::Event &eventParam)
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");

	bool hasEvent = renderWindow->pollEvent(eventParam);
	if (hasEvent)
	{
		switch (eventParam.type)
		{
			case sf::Event::Resized:
			{
				math::VC2U size = math::VC2U(eventParam.size.width, eventParam.size.height);
				screenSizeChangedSignal(size);
			}
			break;
		}
	}
	return hasEvent;
}

bool WindowManager::isInFocus() const
{
	return renderWindow->hasFocus();
}

bool WindowManager::isOpen() const
{
	return renderWindow->isOpen();
}

void WindowManager::setAcceptDropfiles(bool enabled)
{
#if TS_PLATFORM == TS_WINDOWS
	DragAcceptFiles(renderWindow->getSystemHandle(), enabled);
#else
	TS_ASSERT(!"Not implemented on this platform.");
#endif
}

void WindowManager::setMinMaxSize(const math::VC2U &minSizeParam, const math::VC2U &maxSizeParam)
{
	TS_ASSERT(minSizeParam.x <= maxSizeParam.x && minSizeParam.y <= maxSizeParam.y);
	minSize = minSizeParam;
	maxSize = maxSizeParam;
}

math::VC2U WindowManager::getSize() const
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");
	return static_cast<math::VC2U>(renderWindow->getSize());
}

void WindowManager::useApplicationView()
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");
	TS_ASSERT(windowViewManager);
	TS_VERIFY_POINTERS(windowViewManager);

	currentViewType = WindowViewManager::ViewType_Application;
	renderWindow->setView(windowViewManager->getSFMLView(currentViewType));
}

void WindowManager::useInterfaceView()
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");
	TS_ASSERT(windowViewManager);
	TS_VERIFY_POINTERS(windowViewManager);

	currentViewType = WindowViewManager::ViewType_Interface;
	renderWindow->setView(windowViewManager->getSFMLView(currentViewType));
}

const WindowView &WindowManager::getApplicationView() const
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");
	TS_ASSERT(windowViewManager);
	return windowViewManager->getView(WindowViewManager::ViewType_Application);
}

const WindowView &WindowManager::getInterfaceView() const
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");
	TS_ASSERT(windowViewManager);
	return windowViewManager->getView(WindowViewManager::ViewType_Interface);
}

const WindowView &WindowManager::getCurrentView() const
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");
	TS_ASSERT(windowViewManager);
	if (windowViewManager == nullptr)
	{
		static const WindowView empty;
		return empty;
	}
	return windowViewManager->getView(currentViewType);
}

bool WindowManager::systemEventCallback(SystemEventCallbackParams *params)
{
#if TS_PLATFORM == TS_WINDOWS
	switch (params->message)
	{
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO *info = reinterpret_cast<MINMAXINFO *>(params->lParam);
			info->ptMinTrackSize.x = minSize.x;
			info->ptMinTrackSize.y = minSize.y;
			info->ptMaxTrackSize.x = maxSize.x;
			info->ptMaxTrackSize.y = maxSize.y;
			return true; // override default
		}
		break;

		case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)params->wParam;

			// 0xFFFFFFFF as the second parameter returns the count of files dropped
			SizeType numFilesDropped = DragQueryFileW(hDrop, 0xFFFFFFFF, 0, 0);

			TS_PRINTF("numFilesDropped %u\n", numFilesDropped);
			
			if (filesDroppedSignal.hasConnections())
			{
				std::vector<DroppedFile> files;

				wchar_t dropFilename[MAX_PATH];
				for (SizeType index = 0; index < numFilesDropped; ++index)
				{
					if (DragQueryFileW(hDrop, index, dropFilename, MAX_PATH))
					{
						POINT dropPoint;
						memset(&dropPoint, 0, sizeof(dropPoint));
						DragQueryPoint(hDrop, &dropPoint);

						files.push_back({
							String(dropFilename),
							math::VC2I(dropPoint.x, dropPoint.y)
						});
					}
				}

				filesDroppedSignal(files);
			}

			DragFinish(hDrop);
		}
		break;

		case WM_SYSCOMMAND:
		{
			switch ((params->wParam & 0xFFF0))
			{
				case SC_RESTORE:  TS_PRINTF("SC_RESTORE!\n");  break;
				case SC_MAXIMIZE: TS_PRINTF("SC_MAXIMIZE!\n"); break;
				case SC_MINIMIZE: TS_PRINTF("SC_MINIMIZE!\n"); break;
				default: /* bop */ break;
			}
		}
		break;

		default: /* bop */ break;
	}

// 	TS_PRINTF("systemEventCallback message %u\n", params->message);
#endif

	return false;
}

bool WindowManager::setWindowIcon(const String &filepath)
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");

	resource::ResourceManager &rm = getGigaton<resource::ResourceManager>();
	resource::ImageResource *iconResource = rm.reloadResource<resource::ImageResource>(
		"primary_window_icon", filepath, true);
	if (iconResource == nullptr)
		return false;

	sf::Image &icon = *iconResource->getResource();
	renderWindow->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	return true;
}

sf::RenderWindow &WindowManager::getRenderWindow()
{
	TS_ASSERT(renderWindow != nullptr && "Window should be created before using.");
	return *renderWindow;
}

std::vector<math::VC2U> WindowManager::getSupportedResolutions(const bool fullscreen, const math::VC2U &minimumSize)
{
	sf::VideoMode native = sf::VideoMode::getDesktopMode();
	SizeType nativeBits = native.bitsPerPixel;

	std::vector<math::VC2U> result;

	for (sf::VideoMode mode : sf::VideoMode::getFullscreenModes())
	{
		if (!fullscreen && (mode.width == native.width || mode.height == native.height))
			continue;

		if (mode.width < minimumSize.x || mode.height < minimumSize.y)
			continue;

		if (mode.bitsPerPixel != nativeBits)
			continue;

		result.push_back(math::VC2U(mode.width, mode.height));
	}

	float nativeAspect = native.width / (float)native.height;

	// Sort modes based on aspect ratio (nearest to native) and size (largest to smallest)
	std::sort(result.begin(), result.end(),
		[nativeAspect](const math::VC2U &lhs, const math::VC2U &rhs) -> bool
		{
			float lhsRatio = lhs.x / (float)lhs.y;
			float rhsRatio = rhs.x / (float)rhs.y;
			float lhsDiff = math::abs(nativeAspect - lhsRatio);
			float rhsDiff = math::abs(nativeAspect - rhsRatio);
			return lhsDiff < rhsDiff && (lhs.x * lhs.y) > (rhs.x * rhs.y);
		}
	);

	return result;
}

TS_END_PACKAGE1()

