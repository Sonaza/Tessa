#pragma once

#include "ts/tessa/Time/TimeSpan.h"

#define TS_DECLARE_SCENE(__class_name) \
	private: \
		typedef __class_name ThisClass;

TS_DECLARE1(system, BaseApplication);
TS_DECLARE1(resource, ResourceManager);

TS_PACKAGE1(system)

class AbstractSceneBase
{
public:
	AbstractSceneBase(system::BaseApplication *application);
	virtual ~AbstractSceneBase();

	virtual bool start() = 0;
	virtual void stop() = 0;

	virtual void loadResources(resource::ResourceManager &rm) = 0;

	virtual bool handleEvent(const sf::Event event) = 0;
	virtual void update(const TimeSpan deltaTime) = 0;
	virtual void render(sf::RenderWindow &renderWindow) = 0;

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

	friend class system::BaseApplication;
	system::BaseApplication *application = nullptr;
};

template <class T>
TS_FORCEINLINE T &AbstractSceneBase::getGigaton()
{
	return system::Gigaton::getSingleton().getGigaton<T>();
}

template <class T>
TS_FORCEINLINE const T &AbstractSceneBase::getGigaton() const
{
	return system::Gigaton::getSingleton().getGigaton<T>();
}

template <class T>
TS_FORCEINLINE T *AbstractSceneBase::getGigatonOptional()
{
	return system::Gigaton::getSingleton().getGigatonOptional<T>();
}

TS_END_PACKAGE1()
