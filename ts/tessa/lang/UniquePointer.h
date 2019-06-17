#pragma once

TS_PACKAGE1(lang)

class DefaultUniquePointerDeleter;

class UniquePointerBaseImpl
{
public:
	UniquePointerBaseImpl() = default;

protected:
	void *pointer = nullptr;
};

TS_END_PACKAGE1()

TS_PACKAGE0()

// Unique pointer type intended for use cases where a thing is only allowed to
// exist in one place at a time but may be moved elsewhere through move semantics.
template <class T, class Deleter = lang::DefaultUniquePointerDeleter>
class UniquePointer : public lang::UniquePointerBaseImpl
{
public:
	typedef T ElementType;

	UniquePointer() = default;
	explicit UniquePointer(T *pointer);
	~UniquePointer();

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
	void _deleteImpl();
};

#include "UniquePointer.inl"

template <class T, class Deleter = lang::DefaultUniquePointerDeleter, class... Args>
UniquePointer<T, Deleter> makeUnique(Args&&... args)
{
	T *pointer = new T(std::forward<Args>(args)...);
	TS_ASSERT(pointer != nullptr);
	return UniquePointer<T, Deleter>(pointer);
}

TS_END_PACKAGE0()
