#pragma once

#include <atomic>
#include <functional>

TS_PACKAGE1(lang)

class DefaultSharedPointerDeleter;

TS_END_PACKAGE1()

TS_PACKAGE0()

// Shared pointer type intended for use cases where a thing may be shared between several 
// different scopes and at the end when it is no longer needed the allocated memory is freed.
template <class T>
class SharedPointer
{
public:
	typedef T ElementType;

	SharedPointer() = default;
	SharedPointer(nullptr_t);
	explicit SharedPointer(T *ptr);

	template <class Deleter>
	explicit SharedPointer(T *ptr, Deleter);

	// Casting constructor (from another pointer type). Handles copying pointer, refcounter, deleter and does ref increment.
	template <class T2>
	explicit SharedPointer(const SharedPointer<T2> &other, T *ptr);

	~SharedPointer();

	// Copying allowed, increases refcounter.
	SharedPointer(const SharedPointer &other);
	SharedPointer &operator=(const SharedPointer &other);

	// Moving allowed, doesn't increase refcounter, other pointer becomes invalid.
	SharedPointer(SharedPointer &&other) noexcept;
	SharedPointer &operator=(SharedPointer &&other) noexcept;

	T *get() const;

	void reset(T *ptrParam = nullptr);

	template<class Deleter>
	void reset(T *ptrParam, Deleter);

	typename std::add_lvalue_reference<T>::type operator*() const;

	T *operator->();
	const T *operator->() const;

	explicit operator bool() const;
// 	explicit operator void *() const;

	bool operator!() const;
	bool operator==(nullptr_t) const;
	bool operator!=(nullptr_t) const;
	bool operator==(const SharedPointer &other) const;
	bool operator!=(const SharedPointer &other) const;

	template <class T2>
	bool operator==(const SharedPointer<T2> &other) const;
	template <class T2>
	bool operator!=(const SharedPointer<T2> &other) const;
	
	bool isUnique() const;
	void swap(SharedPointer &other);

private:
	void increaseReferenceCounter();
	void decreaseReferenceCounter();

	class SharedPointerImpl
	{
	public:
		std::atomic<Int32> refcount;
		void (*destructor)(void *) = nullptr;
	};

	T *pointer = nullptr;
	SharedPointerImpl *impl = nullptr;

	// Needed to allow type casting in the cast constructor.
	template <class T2>
	friend class SharedPointer;
};

#include "SharedPointer.inl"

template <class T, class... Args>
SharedPointer<T> makeShared(Args&&... args)
{
	T *pointer = new T(std::forward<Args>(args)...);
	TS_ASSERT(pointer != nullptr);
	return SharedPointer<T>(pointer);
}

template <class ToType, class FromType>
SharedPointer<ToType> staticPointerCast(const SharedPointer<FromType> &ptr) noexcept
{
	ToType *castPtr = static_cast<typename SharedPointer<ToType>::ElementType *>(ptr.get());
	return SharedPointer<ToType>(ptr, castPtr);
}

template <class ToType, class FromType>
SharedPointer<ToType> dynamicPointerCast(const SharedPointer<FromType> &ptr) noexcept
{
	ToType *castPtr = dynamic_cast<typename SharedPointer<ToType>::ElementType *>(ptr.get());
	if (castPtr != nullptr)
		return SharedPointer<ToType>(ptr, castPtr);
	else
		return SharedPointer<ToType>();
}

template <class ToType, class FromType>
SharedPointer<ToType> constPointerCast(const SharedPointer<FromType> &ptr) noexcept
{
	ToType *castPtr = const_cast<typename SharedPointer<ToType>::ElementType *>(ptr.get());
	return SharedPointer<ToType>(ptr, castPtr);
}

template <class ToType, class FromType>
SharedPointer<ToType> reinterpretPointerCast(const SharedPointer<FromType> &ptr) noexcept
{
	ToType *castPtr = reinterpret_cast<typename SharedPointer<ToType>::ElementType *>(ptr.get());
	return SharedPointer<ToType>(ptr, castPtr);
}

TS_END_PACKAGE0()
