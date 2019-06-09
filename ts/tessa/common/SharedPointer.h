#pragma once

#include <atomic>
#include <functional>

TS_PACKAGE0()

// Shared pointer type intended for use cases where a thing may be shared between several 
// different scopes and at the end when it is no longer needed the allocated memory is freed.

template<class T>
class SharedPointerDefaultDeleter
{
public:
	void operator()(T *pointer)
	{
		TS_PRINTF("SharedPointerDefaultDeleter called\n");
		delete pointer;
	}
};

template <class T>
class SharedPointer
{
public:
	typedef T ElementType;

	SharedPointer() = default;

	explicit SharedPointer(T *ptr);

	template <class T2>
	explicit SharedPointer(const SharedPointer<T2> &other, T *ptr);

	template <class Deleter>
	explicit SharedPointer(T *ptr, Deleter);

// 	SharedPointer(T *pointer, void(*func(T *)));

	~SharedPointer();

	SharedPointer(const SharedPointer<T> &other);
	SharedPointer<T> &operator=(const SharedPointer<T> &other);

	SharedPointer(SharedPointer<T> &&other);
	SharedPointer<T> &operator=(SharedPointer<T> &&other);

// 	template<class T2>
// 	SharedPointer(SharedPointer<T2> &other);

	T *get() const;

	void reset(T *ptrParam = nullptr);

	template<class Deleter>
	void reset(T *ptrParam, Deleter);

	T &operator*();
	T *operator->();

	explicit operator bool();

	// Implicit cast to void*, it's dirty but fixes pointer verification macro
	operator void *();

	bool operator!() const;
	bool operator==(nullptr_t) const;
	bool operator!=(nullptr_t) const;
	bool operator==(const SharedPointer<T> &other) const;
	bool operator!=(const SharedPointer<T> &other) const;
	
	bool isUnique() const;
	void swap(SharedPointer &other);

private:
	void increaseReferenceCounter();
	void decreaseReferenceCounter();

	class SharedPointerImpl
	{
	public:
		std::atomic<Int32> refcount;
		void (*destructor)(T*) = nullptr;
	};

	T *pointer = nullptr;
	SharedPointerImpl *impl = nullptr;


	template <class T2>
	friend class SharedPointer;
};

#include "SharedPointer.inl"

template <class T, class... Args>
SharedPointer<T> makeShared(Args&&... args)
{
	static_assert(std::is_array<T>::value == false, "SharedPointer does not support storing arrays.");

	T *pointer = new T(args...);
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
