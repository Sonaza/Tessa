#pragma once

TS_PACKAGE0()

// Shared pointer type intended for use cases where a thing may be shared between several 
// different scopes and at the end when it is no longer needed the allocated memory is freed.

template<class T>
class UniquePointerDefaultDeleter
{
public:
	void operator()(T *pointer)
	{
		TS_PRINTF("UniquePointerDefaultDeleter called\n");
		delete pointer;
	}
};

template <class T>
class UniquePointer
{
public:
	typedef T ElementType;

	UniquePointer() = default;

	explicit UniquePointer(T *pointer);

	template <class Deleter>
	explicit UniquePointer(T *pointer, Deleter);

	~UniquePointer();

	UniquePointer(UniquePointer<T> &other) = delete;
	UniquePointer<T> &operator=(UniquePointer<T> &other) = delete;

	UniquePointer(UniquePointer<T> &&other);
	UniquePointer<T> &operator=(UniquePointer<T> &&other);

	T *get() const;

	// Resets pointer without freeing the data
	void dismiss();

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
	bool operator==(const UniquePointer<T> &other) const;
	bool operator!=(const UniquePointer<T> &other) const;
	
	void swap(UniquePointer &other);

private:
	void deletePointerDataIfNeeded();

	void (*destructor)(T*) = nullptr;
	T *pointer = nullptr;
};

#include "UniquePointer.inl"

template <class T, class... Args>
UniquePointer<T> makeUnique(Args&&... args)
{
	static_assert(std::is_array<T>::value == false, "UniquePointer does not support storing arrays.");

	T *pointer = new T(args...);
	TS_ASSERT(pointer != nullptr);
	return UniquePointer<T>(pointer);
}

TS_END_PACKAGE0()
