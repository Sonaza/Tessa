#pragma once

#include "ts/tessa/Time/TimeSpan.h"

#define TS_DECLARE_MANAGER_TYPE(__class_name) \
	private: \
		typedef __class_name ThisClass; \
	public: \
		static const char * TypeName;

#define TS_DEFINE_MANAGER_TYPE(__class_name) \
	namespace ts { const char * __class_name::TypeName = #__class_name; }

TS_PACKAGE1(system)

class AbstractManagerBase : public lang::Noncopyable
{
public:
	AbstractManagerBase();
	virtual ~AbstractManagerBase();

	virtual bool initialize() = 0;
	virtual void deinitialize() = 0;

	virtual void update(const TimeSpan deltaTime) {}

protected:
	template <class T>
	T &getGigaton();

	template <class T>
	const T &getGigaton() const;

	template <class T>
	T *getGigatonOptional();

	system::Gigaton &gigaton;
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

TS_END_PACKAGE1()
