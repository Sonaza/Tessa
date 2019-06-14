#pragma once

#include "ts/tessa/system/SystemManagerBase.h"
#include <SFML/Graphics.hpp>

TS_DECLARE1(system, Application);

TS_PACKAGE1(system)

class WindowManager : public system::SystemManagerBase<TS_FOURCC('W','M','A','N')>
{
	TS_DECLARE_SYSTEM_MANAGER_TYPE(system::WindowManager);

public:
	WindowManager(system::Application *application);
	virtual ~WindowManager();

	virtual bool initialize();
	virtual void deinitialize();

	virtual void update(const sf::Time deltaTime);

	void create(const math::VC2U &videomode, const std::string &windowTitle, const bool resizable, const bool fullscreen);
	void close();

	bool pollEvent(sf::Event &eventParam);

	bool isOpen() const;

	math::VC2U getSize() const;
	math::VC2U getViewSize() const;

	void useGameView();
	void useInterfaceView();

	void setCustomView(sf::View customView);
	void resetView();

	bool setWindowIcon(const std::string &filepath);

	sf::RenderWindow &getRenderWindow();

	std::vector<math::VC2U> getSupportedResolutions(const bool fullscreen, const math::VC2U &minimumSize = math::VC2U::zero);

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
};

TS_END_PACKAGE1()
