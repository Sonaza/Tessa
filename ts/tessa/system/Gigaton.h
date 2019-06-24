#pragma once

#include <unordered_map>
#include <typeinfo>
#include <typeindex>

#define TS_GIGATON_REGISTER_CLASS(instance) ::ts::system::Gigaton::getSingleton().registerClass(instance)
#define TS_GIGATON_UNREGISTER_CLASS(instance) ::ts::system::Gigaton::getSingleton().unregisterClass(instance)

#define TS_GET_GIGATON() ::ts::system::Gigaton::getSingleton()

TS_PACKAGE1(system)

class Gigaton
{
public:
	static Gigaton &getSingleton();

	void unregisterAll();

	template <class T>
	void registerClass(T *instance);

	template <class T>
	void unregisterClass(T *instance);

	template <class T>
	T &getGigaton();

	template <class T>
	const T &getGigaton() const;

	template <class T>
	T *getGigatonOptional();

protected:
	Gigaton();
	~Gigaton();

	std::unordered_map<std::type_index, void *> instances;
};

template <class T>
void Gigaton::registerClass(T *instance)
{
	std::type_index index = typeid(T);
	TS_ASSERT(instances.count(index) == 0 && "An instance of the class is already registered to Gigaton.");
	instances[index] = instance;
}

template <class T>
void Gigaton::unregisterClass(T *instance)
{
	std::type_index index = typeid(T);
	TS_ASSERT(instances.count(index) > 0 && "No instance of the class is registered to Gigaton.");
	instances.erase(index);
}

template <class T>
T &Gigaton::getGigaton()
{
	std::type_index index = typeid(T);
	TS_ASSERT(instances.count(index) > 0 && "No instance of the class is registered to Gigaton.");
	return *static_cast<T*>(instances[index]);
}

template <class T>
const T &Gigaton::getGigaton() const
{
	std::type_index index = typeid(T);
	TS_ASSERT(instances.count(index) > 0 && "No instance of the class is registered to Gigaton.");
	return *static_cast<T*>(instances[index]);
}

template <class T>
T *Gigaton::getGigatonOptional()
{
	std::type_index index = typeid(T);
	if (instances.count(index) > 0)
		return static_cast<T*>(instances[index]);
	return nullptr;
}

TS_END_PACKAGE1()
