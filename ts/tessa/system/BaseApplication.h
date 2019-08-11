#pragma once

#include <type_traits>
#include <unordered_map>

#include "ts/tessa/system/Commando.h"
#include "ts/tessa/system/ConfigReader.h"

#include "ts/tessa/system/SystemManagerBase.h"
#include "ts/tessa/system/AbstractSceneBase.h"

#include "ts/tessa/time/Clock.h"

TS_DECLARE1(system, ThreadPool);
TS_DECLARE1(system, WindowManager);
TS_DECLARE1(file, ArchivistFilesystem);
TS_DECLARE1(resource, ResourceManager);
TS_DECLARE1(resource, FontResource);

TS_PACKAGE1(system)

class BaseApplication
{
public:
	BaseApplication(Int32 argc, const char **argv);
	BaseApplication(Int32 argc, const wchar_t **argv);
	~BaseApplication();

	Int32 launch();

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

protected:
	// Methods for derived Application to implement
	virtual bool start() = 0;
	virtual void stop() = 0;

	virtual void initializeConfigDefaults(system::ConfigReader &config) = 0;
	virtual bool initializeScene() = 0;

	virtual bool createWindow(system::WindowManager &windowManager) = 0;
	virtual bool loadArchives(file::ArchivistFilesystem &fileSystem) = 0;

private:
	bool initialize();
	void deinitialize();

	bool initializeManagers();
	void deinitializeManagers();

	void mainloop();

	void handleEvents();
	void handleUpdate(const TimeSpan deltaTime);
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
	TimeSpan targetFrameTime = TimeSpan::fromMilliseconds(16);
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
bool BaseApplication::createManagerInstance(Args... args)
{
	static_assert(std::is_base_of<system::AbstractSystemManagerBase, ManagerType>::value, "Manager type must inherit from SystemManagerBase.");

	ManagerType *managerInstance = new(std::nothrow) ManagerType(this, args...);
	TS_ASSERT(managerInstance && "Failed to allocate new manager instance.");
	if (managerInstance == nullptr)
		return false;

	if (!managerInstance->initialize())
	{
		TS_LOG_ERROR("Initializing a manager instance failed. Manager type: %s", ManagerType::TypeName);
		return false;
	}

	systemManagers.emplace(ManagerType::TypeIndex, managerInstance);
	return true;
}

template<class ManagerType>
void BaseApplication::destroyManagerInstance()
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
ManagerType &BaseApplication::getManager()
{
	SystemManagersList::iterator it = systemManagers.find(ManagerType::TypeIndex);
	TS_ASSERT(it != systemManagers.end() && "Attempting to retrieve a manager that has not been created.");
	return *static_cast<ManagerType*>(it->second);
}

template<class ManagerType>
const ManagerType &BaseApplication::getManager() const
{
	SystemManagersList::const_iterator it = systemManagers.find(ManagerType::TypeIndex);
	TS_ASSERT(it != systemManagers.end() && "Attempting to retrieve a manager that has not been created.");
	return *static_cast<ManagerType*>(it->second);
}

TS_END_PACKAGE1()

