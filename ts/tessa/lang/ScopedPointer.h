#pragma once

TS_PACKAGE1(lang)

class DefaultScopedPointerDeleter;

class ScopedPointerBaseImpl
{
public:
	ScopedPointerBaseImpl() = default;

protected:
	void *pointer = nullptr;
};

TS_END_PACKAGE1()

TS_PACKAGE0()

// Scoped pointer type intended for use cases where a thing only
// exists in a single scope and will never move outside of it.
template <class T, class Deleter = lang::DefaultScopedPointerDeleter>
class ScopedPointer : public lang::ScopedPointerBaseImpl
{
public:
	typedef T ElementType;

	ScopedPointer() = default;
	explicit ScopedPointer(T *pointer);
	~ScopedPointer();

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
	void _deleteImpl();
};

#include "ScopedPointer.inl"

template <class T, class... Args>
ScopedPointer<T> makeScoped(Args&&... args)
{
	T *pointer = new T(args...);
	TS_ASSERT(pointer != nullptr);
	return ScopedPointer<T>(pointer);
}

TS_END_PACKAGE0()
