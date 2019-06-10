#pragma once

TS_PACKAGE0()

class ScopedPointerDefaultDeleter
{
public:
	void operator()(void *pointer)
	{
		TS_ASSERT(pointer != nullptr);
		delete pointer;
	}
};

// Scoped pointer type intended for use cases where a thing only
// exists in a single scope and will never move outside of it.
template <class T, class Deleter = ScopedPointerDefaultDeleter>
class ScopedPointer
{
public:
	typedef T ElementType;

	ScopedPointer() = default;

	explicit ScopedPointer(T *pointer);

	virtual ~ScopedPointer();

	// Copying is forbidden.
	ScopedPointer(const ScopedPointer &other) = delete;
	ScopedPointer &operator=(const ScopedPointer &other) = delete;

	// Move is forbidden.
	ScopedPointer(ScopedPointer &&other) = delete;
	ScopedPointer &operator=(ScopedPointer &&other) = delete;

	T *get() const;

	// Resets pointer without freeing the data, returns the current raw pointer.
	T *dismiss();

	void reset(T *ptrParam = nullptr);

	typename std::add_lvalue_reference<T>::type operator*() const;
	T *operator->();

	explicit operator bool();
	explicit operator void *();

	bool operator!() const;
	bool operator==(nullptr_t) const;
	bool operator!=(nullptr_t) const;
	bool operator==(const ScopedPointer &other) const;
	bool operator!=(const ScopedPointer &other) const;

	void swap(ScopedPointer &other);

private:
	void deletePointerDataIfNeeded();
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
