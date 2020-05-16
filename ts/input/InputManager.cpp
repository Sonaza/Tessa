#include "Precompiled.h"
#include "InputManager.h"

// #include "ts/profiling/ZoneProfiler.h"
#include "ts/engine/window/WindowManager.h"

TS_DEFINE_MANAGER_TYPE(input::InputManager);

TS_PACKAGE1(input)

struct InputManager::InputDataStorage
{
	math::VC2I mousePosition;
};

InputManager::InputDataStorage &InputManager::getStorage()
{
	static InputDataStorage storage;
	return storage;
}

InputManager::InputManager()
{
	gigaton.registerClass(this);
}

InputManager::~InputManager()
{
	gigaton.unregisterClass(this);
}

bool InputManager::initialize()
{
	return true;
}

void InputManager::deinitialize()
{
	
}

void InputManager::update(const TimeSpan deltaTime)
{
	for (KeyStateMap::iterator it = keyState.begin(); it != keyState.end(); ++it)
	{
		KeyState &state = it->second;
		state.lastPressed = state.pressed;
	}
}

math::VC2I InputManager::getMousePosition()
{
	InputDataStorage &storage = getStorage();
	return storage.mousePosition;
}

bool InputManager::isKeyPressed(Keyboard::Key key) const
{
	return keyState.count(key) > 0 && keyState.at(key).pressed;
}

bool InputManager::wasKeyPressed(Keyboard::Key key) const
{
	return keyState.count(key) > 0 && keyState.at(key).lastPressed == false && keyState.at(key).pressed == true;
}

bool InputManager::wasKeyReleased(Keyboard::Key key) const
{
	return keyState.count(key) > 0 && keyState.at(key).lastPressed == true && keyState.at(key).pressed == false;
}

// bool InputManager::wasKeyPressedWithinPeriod(Keyboard::Key key, TimeSpan period) const
// {
// 	return wasKeyPressed(key) && ((Time::now() - keyState.at(key).lastChanged) <= period);
// }
// 
// bool InputManager::wasKeyReleasedWithinPeriod(Keyboard::Key key, TimeSpan period) const
// {
// 	return wasKeyReleased(key) && ((Time::now() - keyState.at(key).lastChanged) <= period);
// }

bool InputManager::handleEvent(const sf::Event &event)
{
	InputDataStorage &storage = getStorage();

	switch (event.type)
	{
		case sf::Event::KeyPressed:
		{
			Keyboard::Key code = (Keyboard::Key)event.key.code;
			if (keyState[code].lastPressed == false)
			{
				keyState[code].pressed = true;
// 				keyState[code].lastChanged = Time::now();
			}
		}
		break;

		case sf::Event::KeyReleased:
		{
			Keyboard::Key code = (Keyboard::Key)event.key.code;
			if (keyState[code].lastPressed == true)
			{
				keyState[code].pressed = false;
// 				keyState[code].lastChanged = Time::now();
			}
		}
		break;

		case sf::Event::MouseMoved:
		{
			storage.mousePosition = math::VC2I(event.mouseMove.x, event.mouseMove.y);
		}
		break;
		
		default: break;
	}

	return false;
}

TS_END_PACKAGE1()

