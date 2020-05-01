#pragma once

#include "ts/engine/system/AbstractManagerBase.h"

TS_DECLARE2(engine, window, WindowManager);

TS_PACKAGE1(input)

class InputManager : public engine::system::AbstractManagerBase
{
	friend class engine::window::WindowManager;

	TS_DECLARE_MANAGER_TYPE(input::InputManager);

public:
	InputManager();
	virtual ~InputManager();

	virtual bool initialize() override;
	virtual void deinitialize() override;

	static math::VC2I getMousePosition();

private:
	struct InputDataStorage;
	static InputDataStorage &getStorage();

	bool handleEvent(const sf::Event &event);
};

TS_END_PACKAGE1()
