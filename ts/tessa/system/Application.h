#pragma once

#include <SFML/Graphics.hpp>
#include <type_traits>
#include <unordered_map>

#include "ts/tessa/system/Commando.h"
#include "ts/tessa/system/ConfigReader.h"

#include "ts/tessa/system/SystemManagerBase.h"
#include "ts/tessa/system/AbstractSceneBase.h"

TS_DECLARE1(system, ThreadPool);
TS_DECLARE1(system, WindowManager);
TS_DECLARE1(resource, ResourceManager);
TS_DECLARE1(resource, FontResource);

TS_PACKAGE1(system)

class Application
{
public:
	Application(Int32 argc, const char **argv);
	~Application();

	Int32 start();

	void setFramerateLimit(SizeType framerateLimit);
	SizeType getCurrentFramerate() const;

	template<class SceneType>
	bool loadScene();

	template<class ManagerType>
	ManagerType &getManager();

	template<class ManagerType>
	const ManagerType &getManager() const;

	const system::Commando &getCommando() const;
	const system::ConfigReader &getConfig() const;

protected:
	// Abstract methods for GameApplication to implement
	virtual bool sceneInitialize() = 0;
	virtual bool createWindow(system::WindowManager &windowManager) = 0;

private:
	bool initialize();
	void deinitialize();

	bool initializeManagers();
	void deinitializeManagers();

	void mainloop();

	void handleEvents();
	void handleUpdate(const sf::Time deltaTime);
	void handleRendering();

	template<class ManagerType, class... Args>
	bool createManagerInstance(Args... args);

	template<class ManagerType>
	void destroyManagerInstance();

	typedef std::map<SizeType, system::AbstractSystemManagerBase *> SystemManagersList;
	SystemManagersList systemManagers;

	bool applicationRunning = true;

	Commando _commando;
	ConfigReader _config;

	// Target frame time affects framerate, a single update per 16 milliseconds roughly results in 60 fps
	sf::Time targetFrameTime = sf::milliseconds(16);
	SizeType currentFramerate = 0;

	UniquePointer<system::AbstractSceneBase> pendingScene;
	UniquePointer<system::AbstractSceneBase> currentScene;

	resource::FontResource *debugFont = nullptr;

	friend class system::WindowManager;
	friend class resource::ResourceManager;
};

template<class SceneType>
bool Application::loadScene()
{
	static_assert(std::is_base_of<AbstractSceneBase, SceneType>::value, "Registered scene must inherit from SceneBase");

	TS_ASSERT(pendingScene == nullptr && "Another scene is still pending load");
	if (pendingScene != nullptr)
	{
		TS_LOG_ERROR("Cannot load new scene: Another scene is still pending load.");
		return false;
	}

	pendingScene.reset(new SceneType(this));
	if (pendingScene == nullptr)
		return false;

	return true;
}

template<class ManagerType, class... Args>
bool Application::createManagerInstance(Args... args)
{
	static_assert(std::is_base_of<system::AbstractSystemManagerBase, ManagerType>::value, "Manager type must inherit from SystemManagerBase.");

	ManagerType *managerInstance = new(std::nothrow) ManagerType(this, args...);
	TS_ASSERT(managerInstance && "Failed to allocate new manager instance.");
	if (managerInstance == nullptr)
		return false;

	if (!managerInstance->initialize())
	{
		TS_LOG_ERROR("Initializing a manager instance %s failed.", ManagerType::name);
		return false;
	}

	systemManagers.emplace(ManagerType::TypeIndex, managerInstance);
	return true;
}

template<class ManagerType>
void Application::destroyManagerInstance()
{
	static_assert(std::is_base_of<system::AbstractSystemManagerBase, ManagerType>::value, "Manager type must inherit from SystemManagerBase.");

	SystemManagersList::iterator it = systemManagers.find(ManagerType::TypeIndex);
	TS_ASSERT(it != systemManagers.end() && "Attempting to destroy a manager that has not been created.");

	ManagerType *managerInstance = static_cast<ManagerType*>(it->second);
	if (managerInstance != nullptr)
	{
		managerInstance->deinitialize();
		delete managerInstance;
	}
	systemManagers.erase(it);
}

template<class ManagerType>
ManagerType &Application::getManager()
{
	SystemManagersList::iterator it = systemManagers.find(ManagerType::TypeIndex);
	TS_ASSERT(it != systemManagers.end() && "Attempting to retrieve a manager that has not been created.");
	return *static_cast<ManagerType*>(it->second);
}

template<class ManagerType>
const ManagerType &Application::getManager() const
{
	SystemManagersList::const_iterator it = systemManagers.find(ManagerType::TypeIndex);
	TS_ASSERT(it != systemManagers.end() && "Attempting to retrieve a manager that has not been created.");
	return *static_cast<ManagerType*>(it->second);
}

TS_END_PACKAGE1()

