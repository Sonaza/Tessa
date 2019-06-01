#pragma once

#include <SFML/Graphics.hpp>
#include <type_traits>

#include "ts/tessa/system/SceneBase.h"

TS_DECLARE1(system, ThreadPool);
TS_DECLARE1(system, Window);
TS_DECLARE1(resource, ResourceManager);

TS_PACKAGE1(system)

class Application :	public std::enable_shared_from_this<Application>
{
public:
	Application();
	~Application();

	void launch();

	virtual bool sceneInitialize() = 0;

	template<class SceneType>
	bool loadScene();

	void setFramerateLimit(SizeType framerateLimit);
	SizeType getCurrentFramerate() const;

	std::shared_ptr<Window> getWindowPtr() { return window; }

	std::shared_ptr<Application> getApplicationPtr() { return shared_from_this(); }

private:
	void initialize();
	void deinitialize();

	void initializeManagers();
	void initializeWindow();

	void mainloop();

	void handleEvents();
	void handleUpdate(const sf::Time deltaTime);
	void handleRendering();

	bool applicationRunning = true;

	// Target frame time affects framerate, a single update per 16 milliseconds results in 60 fps
	sf::Time targetFrameTime = sf::milliseconds(16);
	SizeType currentFramerate = 0;

	std::unique_ptr<SceneBase> pendingScene;
	std::unique_ptr<SceneBase> currentScene;
	std::unique_ptr<ThreadPool> threadPool;

	std::shared_ptr<Window> window;

	friend class resource::ResourceManager;
	std::shared_ptr<resource::ResourceManager> resourceManager;
};

template<class SceneType>
bool Application::loadScene()
{
	static_assert(std::is_base_of<SceneBase, SceneType>::value, "Registered scene must inherit from SceneBase");

	TS_ASSERT(pendingScene == nullptr && "Another scene is still pending load");
	if (pendingScene != nullptr)
	{
		TS_PRINTF("Cannot load new scene: Another scene is still pending load.");
		return false;
	}

	pendingScene.reset(new SceneType(getApplicationPtr()));
	if (pendingScene == nullptr)
		return false;

	return true;
}

TS_END_PACKAGE1()
