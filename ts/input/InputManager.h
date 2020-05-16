#pragma once

#include "ts/engine/system/AbstractManagerBase.h"

#include "ts/input/Keyboard.h"

#include <map>

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

	virtual void update(const TimeSpan deltaTime) override;

	static math::VC2I getMousePosition();

	bool isKeyPressed(Keyboard::Key key) const;
	bool wasKeyPressed(Keyboard::Key key) const;
	bool wasKeyReleased(Keyboard::Key key) const;

// 	bool isMousePressed(Mouse::Button button) const;

// 	bool wasKeyPressedWithinPeriod(Keyboard::Key key, TimeSpan period) const;
// 	bool wasKeyReleasedWithinPeriod(Keyboard::Key key, TimeSpan period) const;

private:
	struct InputDataStorage;
	static InputDataStorage &getStorage();

	bool handleEvent(const sf::Event &event);

	struct KeyState
	{
		bool pressed = false;
		bool lastPressed = false;
// 		Time lastPressedTime;
// 		Time lastReleasedTime;
	};
	typedef std::map<Keyboard::Key, KeyState> KeyStateMap;
	KeyStateMap keyState;
};

TS_END_PACKAGE1()
