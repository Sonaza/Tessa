#pragma once

#include <type_traits>
#include <unordered_map>

#include "ts/tessa/system/Commando.h"
#include "ts/tessa/system/ConfigReader.h"

#include "ts/tessa/system/AbstractManagerBase.h"
#include "ts/tessa/system/AbstractSceneBase.h"

#include "ts/tessa/time/Clock.h"

#include "ts/tessa/profiling/ZoneProfiler.h"

TS_DECLARE1(system, ThreadPool);
TS_DECLARE1(system, WindowManager);
TS_DECLARE1(file, ArchivistFilesystem);
TS_DECLARE1(resource, ResourceManager);
TS_DECLARE1(resource, FontResource);

TS_PACKAGE1(system)

class BaseApplication
{
public:
	BaseApplication(int32 argc, const char **argv);
	BaseApplication(int32 argc, const wchar_t **argv);
	~BaseApplication();

	int32 launch();

	void setFramerateLimit(SizeType framerateLimit);
	SizeType getCurrentFramerate() const;

	template<class SceneType>
	bool loadScene();

	template<class ManagerType>
	ManagerType &getManager();

	template<class ManagerType>
	const ManagerType &getManager() const;

	const system::Commando &getCommando() const;

	system::ConfigReader &getConfig();
	const system::ConfigReader &getConfig() const;

	sf::Font &getDebugFont();

protected:
	// Methods for derived Application to implement
	virtual bool start() = 0;
	virtual void stop() = 0;

	virtual bool createApplicationManagers() = 0;

	virtual void initializeConfigDefaults(system::ConfigReader &config) = 0;
	virtual bool initializeScene() = 0;

	virtual bool createWindow(system::WindowManager &windowManager) = 0;
	virtual bool loadArchives(file::ArchivistFilesystem &fileSystem) = 0;

	virtual bool customQuitHandler() = 0;

	// For System and Application manager instantiation
	template<class ManagerType, class... Args>
	bool createManagerInstance(Args&&... args);


private:
	bool quit();
	void fastExit();

	bool initialize();
	void deinitialize();

	void mainloop();

	void handleEvents();
	void handleRendering();

	bool createSystemManagers();
	void destroyManagerInstances();

	typedef std::map<std::type_index, UniquePointer<system::AbstractManagerBase>> InstancedManagersList;
	InstancedManagersList managerInstances;
	std::vector<std::type_index> managerInstancingOrder;

	bool applicationRunning = true;
	bool showFPS = true;
	
	system::Gigaton &gigaton;

	Commando _commando;
	ConfigReader _config;

	// Fixed delta of all logic updates, independent of frame rate
	const TimeSpan fixedDeltaTime = TimeSpan::fromMilliseconds(16);
	// Target frame time affects framerate, a single update per 8 milliseconds roughly results in 120 fps
	TimeSpan targetFrameTime = TimeSpan::fromMilliseconds(8);
	SizeType currentFramerate = 0;

	UniquePointer<system::AbstractSceneBase> pendingScene;
	UniquePointer<system::AbstractSceneBase> currentScene;

	resource::FontResource *debugFont = nullptr;

	friend class system::WindowManager;
	friend class resource::ResourceManager;
};

template<class SceneType>
bool BaseApplication::loadScene()
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
bool BaseApplication::createManagerInstance(Args&&... args)
{
	static_assert(std::is_base_of<system::AbstractManagerBase, ManagerType>::value, "Manager type must inherit from AbstractManagerBase.");

	const std::type_index typeIndex = typeid(ManagerType);
	if (managerInstances.find(typeIndex) != managerInstances.end())
	{
		TS_ASSERTF(false, "Manager of the type '%s' is already created.", ManagerType::TypeName);
		return false;
	}

	UniquePointer<ManagerType> managerInstance = makeUnique<ManagerType>(std::forward<Args>(args)...);
	TS_ASSERT(managerInstance && "Failed to allocate new manager instance.");
	if (managerInstance == nullptr)
		return false;

	managerInstances.emplace(typeIndex, staticUniquePointerCast<AbstractManagerBase>(std::move(managerInstance)));
	managerInstancingOrder.push_back(typeIndex);
	return true;
}

template<class ManagerType>
ManagerType &BaseApplication::getManager()
{
	TS_ZONE();

	const std::type_index typeIndex = typeid(ManagerType);
	InstancedManagersList::iterator it = managerInstances.find(typeIndex);
	TS_ASSERT(it != managerInstances.end() && "Attempting to retrieve a manager that has not been created.");
	return *static_cast<ManagerType*>(it->second.get());
}

template<class ManagerType>
const ManagerType &BaseApplication::getManager() const
{
	TS_ZONE();

	const std::type_index typeIndex = typeid(ManagerType);
	InstancedManagersList::iterator it = managerInstances.find(typeIndex);
	TS_ASSERT(it != managerInstances.end() && "Attempting to retrieve a manager that has not been created.");
	return *static_cast<ManagerType*>(it->second.get());
}

TS_END_PACKAGE1()

