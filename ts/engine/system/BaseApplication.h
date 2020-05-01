#pragma once

#include <type_traits>
#include <unordered_map>

#include "ts/engine/system/Commando.h"
#include "ts/engine/system/ConfigReader.h"

#include "ts/engine/system/AbstractManagerBase.h"
#include "ts/engine/system/AbstractSceneBase.h"

#include "ts/profiling/ZoneProfiler.h"

TS_DECLARE2(engine, window, WindowManager);
TS_DECLARE2(resource, archivist, ArchivistFilesystem);
TS_DECLARE1(resource, ResourceManager);
TS_DECLARE1(resource, FontResource);

TS_PACKAGE2(engine, system)

class BaseApplication
{
	friend class engine::window::WindowManager;
	friend class resource::ResourceManager;

public:
	BaseApplication(system::Commando &commando);
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

	virtual bool createWindow(window::WindowManager &windowManager) = 0;
	virtual bool loadArchives(resource::archivist::ArchivistFilesystem &fileSystem) = 0;

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
	InstancedManagersList m_managerInstances;
	std::vector<std::type_index> m_managerInstancingOrder;

	bool m_applicationRunning = true;
	bool m_showFramesPerSecond = true;
	
	Gigaton &m_gigatonInstance;

	Commando m_commando;
	ConfigReader m_config;

	// Fixed delta of all logic updates, independent of frame rate
	const TimeSpan m_fixedDeltaTime = TimeSpan::fromMilliseconds(16);
	// Target frame time affects framerate, a single update per 8 milliseconds roughly results in 120 fps
	TimeSpan m_targetFrameTime = TimeSpan::fromMilliseconds(8);
	SizeType m_currentFramerate = 0;

	UniquePointer<system::AbstractSceneBase> m_currentScene;
	UniquePointer<system::AbstractSceneBase> m_pendingScene;

	resource::FontResource *m_debugFont = nullptr;
};

template<class SceneType>
bool BaseApplication::loadScene()
{
	static_assert(std::is_base_of<AbstractSceneBase, SceneType>::value, "Registered scene must inherit from SceneBase");

	TS_ASSERT(m_pendingScene == nullptr && "Another scene is still pending load");
	if (m_pendingScene != nullptr)
	{
		TS_LOG_ERROR("Cannot load new scene: Another scene is still pending load.");
		return false;
	}

	m_pendingScene.reset(new SceneType(this));
	if (m_pendingScene == nullptr)
		return false;

	return true;
}

template<class ManagerType, class... Args>
bool BaseApplication::createManagerInstance(Args&&... args)
{
	static_assert(std::is_base_of<system::AbstractManagerBase, ManagerType>::value, "Manager type must inherit from AbstractManagerBase.");

	const std::type_index typeIndex = typeid(ManagerType);
	if (m_managerInstances.find(typeIndex) != m_managerInstances.end())
	{
		TS_ASSERTF(false, "Manager of the type '%s' is already created.", ManagerType::TypeName);
		return false;
	}

	UniquePointer<ManagerType> managerInstance = makeUnique<ManagerType>(std::forward<Args>(args)...);
	TS_ASSERT(managerInstance && "Failed to allocate new manager instance.");
	if (managerInstance == nullptr)
		return false;

	m_managerInstances.emplace(typeIndex, staticUniquePointerCast<AbstractManagerBase>(std::move(managerInstance)));
	m_managerInstancingOrder.push_back(typeIndex);
	return true;
}

template<class ManagerType>
ManagerType &BaseApplication::getManager()
{
	TS_ZONE();

	const std::type_index typeIndex = typeid(ManagerType);
	InstancedManagersList::iterator it = m_managerInstances.find(typeIndex);
	TS_ASSERT(it != m_managerInstances.end() && "Attempting to retrieve a manager that has not been created.");
	return *static_cast<ManagerType*>(it->second.get());
}

template<class ManagerType>
const ManagerType &BaseApplication::getManager() const
{
	TS_ZONE();

	const std::type_index typeIndex = typeid(ManagerType);
	InstancedManagersList::const_iterator it = m_managerInstances.find(typeIndex);
	TS_ASSERT(it != m_managerInstances.end() && "Attempting to retrieve a manager that has not been created.");
	return *static_cast<ManagerType*>(it->second.get());
}

TS_END_PACKAGE2()

