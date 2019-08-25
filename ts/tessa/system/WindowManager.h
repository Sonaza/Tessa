#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

TS_DECLARE1(system, BaseApplication);

TS_PACKAGE1(system)

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

	void setVSyncEnabled(const bool enabled);

	bool pollEvent(sf::Event &eventParam);

	bool isOpen() const;

	math::VC2U getSize() const;
	math::VC2U getViewSize() const;

	void useApplicationView();
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

	bool windowCreated = false;

	sf::RenderWindow renderWindow;
	sf::View activeApplicationView;
	sf::View activeInterfaceView;
};

TS_END_PACKAGE1()
