#include "Precompiled.h"
#include "InputManager.h"

// #include "ts/tessa/profiling/ZoneProfiler.h"
#include "ts/tessa/system/WindowManager.h"

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
	windowManager = gigaton.getGigatonOptional<system::WindowManager>();
	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(false, windowManager);

	return true;
}

void InputManager::deinitialize()
{
	
}

math::VC2I InputManager::getMousePosition()
{
	InputDataStorage &storage = getStorage();
	return storage.mousePosition;
}

bool InputManager::handleEvent(const sf::Event &event)
{
	InputDataStorage &storage = getStorage();

	switch (event.type)
	{
		case sf::Event::MouseMoved:
		{
			storage.mousePosition = math::VC2I(event.mouseMove.x, event.mouseMove.y);
		}
		break;
	}

	return false;
}

TS_END_PACKAGE1()

