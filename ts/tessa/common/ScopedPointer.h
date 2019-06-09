#pragma once

TS_PACKAGE0()

// Shared pointer type intended for use cases where a thing may be shared between several 
// different scopes and at the end when it is no longer needed the allocated memory is freed.

template<class T>
class ScopedPointerDefaultDeleter
{
public:
	void operator()(T *pointer)
	{
		TS_PRINTF("ScopedPointerDefaultDeleter called\n");
		delete pointer;
	}
};

template <class T>
class ScopedPointer
{
public:
	typedef T ElementType;

	ScopedPointer() = default;

	explicit ScopedPointer(T *pointer);

	template <class Deleter>
	explicit ScopedPointer(T *pointer, Deleter);

	~ScopedPointer();

	ScopedPointer(ScopedPointer<T> &other) = delete;
	ScopedPointer<T> &operator=(ScopedPointer<T> &other) = delete;

	ScopedPointer(ScopedPointer<T> &&other) = delete;
	ScopedPointer<T> &operator=(ScopedPointer<T> &&other) = delete;

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
	bool operator==(const ScopedPointer<T> &other) const;
	bool operator!=(const ScopedPointer<T> &other) const;
	
	void swap(ScopedPointer &other);

private:
	void deletePointerDataIfNeeded();

	void (*destructor)(T*) = nullptr;
	T *pointer = nullptr;
};

#include "ScopedPointer.inl"

template <class T, class... Args>
ScopedPointer<T> makeScoped(Args&&... args)
{
	static_assert(std::is_array<T>::value == false, "ScopedPointer does not support storing arrays.");

	T *pointer = new T(args...);
	TS_ASSERT(pointer != nullptr);
	return ScopedPointer<T>(pointer);
}

TS_END_PACKAGE0()
