#pragma once

#define TS_DECLARE_SYSTEM_MANAGER_TYPE(__class_name) \
	public: \
		static const char * TypeName;

#define TS_DEFINE_SYSTEM_MANAGER_TYPE(__class_name) \
	namespace ts { const char * __class_name::TypeName = #__class_name; }

TS_DECLARE1(system, BaseApplication);

TS_PACKAGE1(system)

class AbstractSystemManagerBase : public lang::Noncopyable
{
public:
	AbstractSystemManagerBase() {}
	virtual ~AbstractSystemManagerBase() {}

	virtual bool initialize() = 0;
	virtual void deinitialize() = 0;

	virtual void update(const sf::Time deltaTime) = 0;

protected:
	template <class T>
	T &getGigaton();

	template <class T>
	const T &getGigaton() const;

	template <class T>
	T *getGigatonOptional();
};

template<SizeType ManagerTypeIndex>
class SystemManagerBase : public AbstractSystemManagerBase
{
public:
	enum { TypeIndex = ManagerTypeIndex };

	SystemManagerBase(system::BaseApplication *application)
		: application(application)
	{
	}
	virtual ~SystemManagerBase() {}

protected:
	system::BaseApplication *application = nullptr;
};

template <class T>
TS_FORCEINLINE T &AbstractSystemManagerBase::getGigaton()
{
	return system::Gigaton::getSingleton().getGigaton<T>();
}

template <class T>
TS_FORCEINLINE const T &AbstractSystemManagerBase::getGigaton() const
{
	return system::Gigaton::getSingleton().getGigaton<T>();
}

template <class T>
TS_FORCEINLINE T *AbstractSystemManagerBase::getGigatonOptional()
{
	return system::Gigaton::getSingleton().getGigatonOptional<T>();
}

TS_END_PACKAGE1()
