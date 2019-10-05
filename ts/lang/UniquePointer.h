#pragma once

TS_PACKAGE1(lang)

class DefaultUniquePointerDeleter;

TS_END_PACKAGE1()

TS_PACKAGE0()

// Unique pointer type intended for use cases where a thing is only allowed to
// exist in one place at a time but may be moved elsewhere through move semantics.
template <class T, class Deleter = lang::DefaultUniquePointerDeleter>
class UniquePointer
{
public:
	typedef T ElementType;

	UniquePointer() = default;
	UniquePointer(nullptr_t);
	explicit UniquePointer(T *pointer);
	~UniquePointer();

	// Copying is forbidden.
	UniquePointer(const UniquePointer &other) = delete;
	UniquePointer &operator=(const UniquePointer &other) = delete;

	// Move is allowed.
	UniquePointer(UniquePointer &&other) noexcept;
	UniquePointer &operator=(UniquePointer &&other) noexcept;

	T *get() const;

	// Resets pointer without freeing the data, returns the current raw pointer.
	T *dismiss();

	void reset(T *ptrParam = nullptr);

	typename std::add_lvalue_reference<T>::type operator*() const;
	
	T *operator->();
	const T *operator->() const;

	explicit operator bool() const;
	explicit operator void *() const;

	bool operator!() const;
	bool operator==(nullptr_t) const;
	bool operator!=(nullptr_t) const;
	bool operator==(const UniquePointer &other) const;
	bool operator!=(const UniquePointer &other) const;
	
	void swap(UniquePointer &other);

private:
	void _deleteImpl();
	T *pointer = nullptr;
};

#include "UniquePointer.inl"

template <class T, class Deleter = lang::DefaultUniquePointerDeleter, class... Args>
UniquePointer<T, Deleter> makeUnique(Args&&... args)
{
	T *pointer = new T(std::forward<Args>(args)...);
	TS_ASSERT(pointer != nullptr);
	return UniquePointer<T, Deleter>(pointer);
}

// Special unique pointer caster that MOVES the original pointer to new while casting it (invalidates old pointer)
template <class ToType, class FromType, class Deleter>
UniquePointer<ToType, Deleter> staticUniquePointerCast(UniquePointer<FromType, Deleter> &&ptr) noexcept
{
	ToType *castPtr = static_cast<typename UniquePointer<ToType, Deleter>::ElementType *>(ptr.dismiss());
	return UniquePointer<ToType, Deleter>(castPtr);
}

TS_END_PACKAGE0()
