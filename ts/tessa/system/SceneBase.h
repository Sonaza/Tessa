#pragma once

TS_DECLARE1(system, Application);
TS_DECLARE1(resource, ResourceManager);

TS_PACKAGE1(system)

class SceneBase
{
public:
	SceneBase(std::shared_ptr<system::Application> application);
	virtual ~SceneBase();

	virtual bool start() = 0;
	virtual void stop() = 0;

	virtual void loadResources(std::shared_ptr<resource::ResourceManager> rm) = 0;

	virtual bool handleEvent(const sf::Event event) = 0;
	virtual void update(const sf::Time deltaTime) = 0;
	virtual void render(sf::RenderWindow &renderWindow) = 0;

	bool isLoaded() const { return sceneLoaded; }

protected:
	friend class system::Application;

	bool internalStart();
	void internalStop();

	bool sceneLoaded = false;

	std::shared_ptr<system::Application> application;
};

TS_END_PACKAGE1()
