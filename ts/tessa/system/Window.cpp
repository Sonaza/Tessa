#include "Precompiled.h"
#include "ts/tessa/system/Window.h"

TS_PACKAGE1(system)

Window::Window(std::shared_ptr<system::Application> application)
	: application(application)
{
}

Window::~Window()
{
}

void Window::create(const math::VC2U &videomode, const std::string &windowTitle)
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

void Window::close()
{
	renderWindow.close();
}

bool Window::pollEvent(sf::Event &eventParam)
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

bool Window::isOpen() const
{
	return renderWindow.isOpen();
}

math::VC2U Window::getSize() const
{
	const sf::Vector2u s = renderWindow.getSize();
	return math::VC2U(s.x, s.y);
}

void Window::useGameView()
{
	renderWindow.setView(activeGameView);
}

void Window::useInterfaceView()
{
	renderWindow.setView(activeInterfaceView);
}

sf::RenderWindow &Window::getRenderWindow()
{
	return renderWindow;
}

TS_END_PACKAGE1()
