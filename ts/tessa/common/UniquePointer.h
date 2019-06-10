#pragma once

TS_PACKAGE0()

class UniquePointerDefaultDeleter
{
public:
	void operator()(void *pointer)
	{
		TS_ASSERT(pointer != nullptr);
		delete pointer;
	}
};

// Unique pointer type intended for use cases where a thing is only allowed to
// exist in one place at a time but may be moved elsewhere through move semantics.
template <class T, class Deleter = UniquePointerDefaultDeleter>
class UniquePointer
{
public:
	typedef T ElementType;

	UniquePointer() = default;

	explicit UniquePointer(T *pointer);

	virtual ~UniquePointer();

	// Copying is forbidden.
	UniquePointer(const UniquePointer &other) = delete;
	UniquePointer &operator=(const UniquePointer &other) = delete;

	// Move is allowed.
	UniquePointer(UniquePointer &&other);
	UniquePointer &operator=(UniquePointer &&other);

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
	bool operator==(const UniquePointer &other) const;
	bool operator!=(const UniquePointer &other) const;
	
	void swap(UniquePointer &other);

private:
	void deletePointerDataIfNeeded();
	T *pointer = nullptr;
};

#include "UniquePointer.inl"

template <class T, class Deleter = UniquePointerDefaultDeleter, class... Args>
UniquePointer<T, Deleter> makeUnique(Args&&... args)
{
	static_assert(std::is_array<T>::value == false, "UniquePointer does not support storing arrays.");

	T *pointer = new T(std::forward<Args>(args)...);
	TS_ASSERT(pointer != nullptr);
	return UniquePointer<T, Deleter>(pointer);
}

TS_END_PACKAGE0()
