#pragma once

#include "ts/engine/Gigaton.h"

#define TS_DECLARE_MANAGER_TYPE(__class_name) \
	private: \
		typedef __class_name ThisClass; \
	public: \
		static const char * TypeName; \
		virtual const char *getTypeName() const override { return ThisClass::TypeName; }

#define TS_DEFINE_MANAGER_TYPE(__class_name) \
	namespace ts { const char * __class_name::TypeName = #__class_name; }

TS_PACKAGE2(engine, system)

class AbstractManagerBase : public lang::Noncopyable
{
	friend class BaseApplication;

public:
	AbstractManagerBase();
	virtual ~AbstractManagerBase();

	virtual bool initialize() = 0;
	virtual void deinitialize() = 0;

	bool isInitialized() const;

	virtual void update(const TimeSpan deltaTime) {}

	virtual const char *getTypeName() const = 0;

protected:
	template <class T>
	T &getGigaton();

	template <class T>
	const T &getGigaton() const;

	template <class T>
	T *getGigatonOptional();

	Gigaton &gigaton;

private:
	bool initialized = false;
};

template <class T>
TS_FORCEINLINE T &AbstractManagerBase::getGigaton()
{
	return gigaton.getGigaton<T>();
}

template <class T>
TS_FORCEINLINE const T &AbstractManagerBase::getGigaton() const
{
	return gigaton.getGigaton<T>();
}

template <class T>
TS_FORCEINLINE T *AbstractManagerBase::getGigatonOptional()
{
	return gigaton.getGigatonOptional<T>();
}

TS_END_PACKAGE2()
