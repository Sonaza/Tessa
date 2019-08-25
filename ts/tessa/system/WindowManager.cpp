#include "Precompiled.h"
#include "ts/tessa/system/WindowManager.h"

TS_DEFINE_MANAGER_TYPE(system::WindowManager);

TS_PACKAGE1(system)

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
	return true;
}

void WindowManager::deinitialize()
{
	close();
}

void WindowManager::update(const TimeSpan deltaTime)
{

}

void WindowManager::create(const math::VC2U &videomode, const std::string &windowTitle, const bool resizable, const bool fullscreen)
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
// 	settings.sRgbCapable = true;

	renderWindow.create(sf::VideoMode(videomode.x, videomode.y), windowTitle, style, settings);
	activeApplicationView = renderWindow.getView();
	activeInterfaceView = renderWindow.getView();

	math::VC2 center = activeInterfaceView.getCenter();
	TS_PRINTF("Center %0.2f, %0.2f\n", center.x, center.y);

	renderWindow.clear();
	renderWindow.display();

	windowCreated = true;
}

void WindowManager::close()
{
	windowCreated = false;
	renderWindow.close();
}

void WindowManager::setVSyncEnabled(const bool enabled)
{
	TS_ASSERT(windowCreated && "Window should be created before using.");
	renderWindow.setVerticalSyncEnabled(enabled);
}

bool WindowManager::pollEvent(sf::Event &eventParam)
{
	TS_ASSERT(windowCreated && "Window should be created before using.");

	bool hasEvent = renderWindow.pollEvent(eventParam);
	if (hasEvent)
	{
		switch (eventParam.type)
		{
			case sf::Event::Resized:
			{
				math::VC2 size((float)eventParam.size.width, (float)eventParam.size.height);

				activeApplicationView.setSize(size);
				activeApplicationView.setCenter(size / 2.f);

				activeInterfaceView.setSize(size);
				activeInterfaceView.setCenter(size / 2.f);
			}
			break;
		}
	}
	return hasEvent;
}

bool WindowManager::isOpen() const
{
	return renderWindow.isOpen();
}

math::VC2U WindowManager::getSize() const
{
	TS_ASSERT(windowCreated && "Window should be created before using.");
	return static_cast<math::VC2U>(renderWindow.getSize());
}

void WindowManager::useApplicationView()
{
	TS_ASSERT(windowCreated && "Window should be created before using.");
	renderWindow.setView(activeApplicationView);
}

void WindowManager::useInterfaceView()
{
	TS_ASSERT(windowCreated && "Window should be created before using.");
	renderWindow.setView(activeInterfaceView);
}

void WindowManager::setCustomView(sf::View customView)
{

}

void WindowManager::resetView()
{

}

bool WindowManager::setWindowIcon(const std::string &filepath)
{
	TS_ASSERT(windowCreated && "Window should be created before using.");

	sf::Image icon;
	if (!icon.loadFromFile(filepath))
	{
		TS_LOG_ERROR("Unable to set window icon, file load failed. File: %s", filepath);
		return false;
	}
	renderWindow.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	return true;
}

sf::RenderWindow &WindowManager::getRenderWindow()
{
	TS_ASSERT(windowCreated && "Window should be created before using.");
	return renderWindow;
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

// 	for (auto r : result)
// 	{
// 		auto ratio = r / math::greatestCommonDivisor(r.x, r.y);
// 		if (ratio.x == 8) ratio *= 2U;
// 		TS_PRINTF("%u, %u (%u:%u)\n", r.x, r.y, ratio.x, ratio.y);
// 	}

	return result;
}

TS_END_PACKAGE1()
