#pragma once

#include "ts/time/TimeSpan.h"

#define TS_DECLARE_SCENE(__class_name) \
	private: \
		typedef __class_name ThisClass;

TS_DECLARE2(engine, system, BaseApplication);
TS_DECLARE1(resource, ResourceManager);

TS_DECLARE_STRUCT2(engine, window, WindowView);

TS_PACKAGE2(engine, system)

class AbstractSceneBase
{
public:
	AbstractSceneBase(engine::system::BaseApplication *application);
	virtual ~AbstractSceneBase();

	virtual bool start() = 0;
	virtual void stop() = 0;

	virtual void loadResources(resource::ResourceManager &rm) = 0;

	virtual bool handleEvent(const sf::Event event) = 0;

	virtual void update(const TimeSpan deltaTime) = 0;
	virtual void updateFrequent(const TimeSpan deltaTime) = 0;

	virtual void renderApplication(sf::RenderTarget &renderTarget, const engine::window::WindowView &view) = 0;
	virtual void renderInterface(sf::RenderTarget &renderTarget, const engine::window::WindowView &view) = 0;

	bool isLoaded() const { return sceneLoaded; }

protected:
	template <class T>
	T &getGigaton();

	template <class T>
	const T &getGigaton() const;

	template <class T>
	T *getGigatonOptional();

	bool internalStart();
	void internalStop();

	bool sceneLoaded = false;

	friend class engine::system::BaseApplication;
	engine::system::BaseApplication *application = nullptr;

private:
	Gigaton &gigaton;
};

template <class T>
TS_FORCEINLINE T &AbstractSceneBase::getGigaton()
{
	return gigaton.getGigaton<T>();
}

template <class T>
TS_FORCEINLINE const T &AbstractSceneBase::getGigaton() const
{
	return gigaton.getGigaton<T>();
}

template <class T>
TS_FORCEINLINE T *AbstractSceneBase::getGigatonOptional()
{
	return gigaton.getGigatonOptional<T>();
}

TS_END_PACKAGE2()
