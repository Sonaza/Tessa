#include "Precompiled.h"
#include "ts/tessa/system/WindowManager.h"

TS_DEFINE_SYSTEM_MANAGER_TYPE(system::WindowManager);

TS_PACKAGE1(system)

WindowManager::WindowManager(system::Application *application)
	: SystemManagerBase(application)
{
}

WindowManager::~WindowManager()
{
}

bool WindowManager::initialize()
{
	return true;
}

void WindowManager::deinitialize()
{
	close();
}

void WindowManager::create(const math::VC2U &videomode, const std::string &windowTitle)
{
	uint32_t style = sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close;

	sf::ContextSettings settings;
	settings.antialiasingLevel = 2;

	renderWindow.create(sf::VideoMode(videomode.x, videomode.y), windowTitle, style, settings);
	activeGameView = renderWindow.getView();
	activeInterfaceView = renderWindow.getView();

	renderWindow.clear();
	renderWindow.display();
}

void WindowManager::close()
{
	renderWindow.close();
}

bool WindowManager::pollEvent(sf::Event &eventParam)
{
	bool hasEvent = renderWindow.pollEvent(eventParam);
	if (hasEvent)
	{
		switch (eventParam.type)
		{
			case sf::Event::Resized:
			{
				activeInterfaceView.setSize((float)eventParam.size.width, (float)eventParam.size.height);
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
	const sf::Vector2u s = renderWindow.getSize();
	return math::VC2U(s.x, s.y);
}

void WindowManager::useGameView()
{
	renderWindow.setView(activeGameView);
}

void WindowManager::useInterfaceView()
{
	renderWindow.setView(activeInterfaceView);
}

sf::RenderWindow &WindowManager::getRenderWindow()
{
	return renderWindow;
}

TS_END_PACKAGE1()
