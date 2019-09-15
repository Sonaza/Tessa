#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

TS_DECLARE1(system, WindowManager);

TS_PACKAGE1(input)

class InputManager : public system::AbstractManagerBase
{
	friend class system::WindowManager;

	TS_DECLARE_MANAGER_TYPE(input::InputManager);

public:
	InputManager();
	virtual ~InputManager();

	virtual bool initialize();
	virtual void deinitialize();

	static math::VC2I getMousePosition();

private:
	struct InputDataStorage;
	static InputDataStorage &getStorage();

	bool handleEvent(const sf::Event &event);

	system::WindowManager *windowManager = nullptr;
};

TS_END_PACKAGE1()
