#pragma once

#include <SFML/Graphics.hpp>

TS_DECLARE1(system, Application);

TS_PACKAGE1(system)

class Window
{
public:
	Window(std::shared_ptr<system::Application> application);
	~Window();

	void create(const math::VC2U &videomode, const std::string &windowTitle);
	void close();

	bool pollEvent(sf::Event &eventParam);

	bool isOpen() const;

	math::VC2U getSize() const;
	math::VC2U getViewSize() const;

	void useGameView();
	void useInterfaceView();

	void setCustomView(sf::View customView);
	void resetView();

	sf::RenderWindow &getRenderWindow();

private:
// 	struct Settings
// 	{
// 		std::string title;
// 		Uint32 style = sf::Style::Default;
// 		sf::ContextSettings context;
// 		math::VC2U size;
// 	};
// 	Settings settings;

	sf::RenderWindow renderWindow;
	sf::View activeGameView;
	sf::View activeInterfaceView;

	friend class Application;
	std::shared_ptr<system::Application> application;
};

TS_END_PACKAGE1()
