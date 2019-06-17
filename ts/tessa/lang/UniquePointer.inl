
class lang::DefaultUniquePointerDeleter
{
public:
	template <class T>
	void operator()(T *pointer)
	{
		// Type completeness check copied from boost's checked delete.
		// intentionally complex - simplification causes regressions
		typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
		(void) sizeof(type_must_be_complete);
		delete pointer;
	}
};

template <class T, class Deleter>
UniquePointer<T, Deleter>::UniquePointer(T *pointer)
{
	reset(pointer);
}

template <class T, class Deleter>
UniquePointer<T, Deleter>::~UniquePointer()
{
	reset();
}

template <class T, class Deleter>
UniquePointer<T, Deleter>::UniquePointer(UniquePointer &&other)
{
	*this = std::move(other);
}

template <class T, class Deleter>
UniquePointer<T, Deleter> &UniquePointer<T, Deleter>::operator=(UniquePointer &&other)
{
	// Prevent self assignment
	if (this != &other)
	{
		_deleteImpl();
		pointer = other.pointer;
		other.pointer = nullptr;
	}
	return *this;
}

template <class T, class Deleter>
T *UniquePointer<T, Deleter>::get() const
{
	return static_cast<T *>(pointer);
}

template <class T, class Deleter>
T *UniquePointer<T, Deleter>::dismiss()
{
	T *ptr = static_cast<T *>(pointer);
	pointer = nullptr;
	return ptr;
}

template <class T, class Deleter>
void UniquePointer<T, Deleter>::reset(T *ptrParam)
{
	_deleteImpl();
	pointer = ptrParam;
}

template <class T, class Deleter>
typename std::add_lvalue_reference<T>::type UniquePointer<T, Deleter>::operator*() const
{
	TS_ASSERTF(pointer != nullptr, "Attempting to dereference a null pointer");
	return *static_cast<T *>(pointer);
}

template <class T, class Deleter>
T *UniquePointer<T, Deleter>::operator->()
{
	TS_ASSERTF(pointer != nullptr, "Attempting indirection on a null pointer");
	return static_cast<T *>(pointer);
}

template <class T, class Deleter>
UniquePointer<T, Deleter>::operator bool()
{
	return pointer != nullptr;
}

template <class T, class Deleter>
UniquePointer<T, Deleter>::operator void *()
{
	return static_cast<void*>(pointer);
}

template <class T, class Deleter>
bool UniquePointer<T, Deleter>::operator!() const
{
	return pointer == nullptr;
}

template <class T, class Deleter>
bool UniquePointer<T, Deleter>::operator==(nullptr_t) const
{
	return pointer == nullptr;
}

template <class T, class Deleter>
bool UniquePointer<T, Deleter>::operator!=(nullptr_t) const
{
	return pointer != nullptr;
}

template <class T, class Deleter>
bool UniquePointer<T, Deleter>::operator==(const UniquePointer &other) const
{
	return pointer == other.pointer;
}

template <class T, class Deleter>
bool UniquePointer<T, Deleter>::operator!=(const UniquePointer &other) const
{
	return pointer != other.pointer;
}

template <class T, class Deleter>
void UniquePointer<T, Deleter>::swap(UniquePointer &other)
{
	std::swap(pointer, other.pointer);
}

template <class T, class Deleter>
void UniquePointer<T, Deleter>::_deleteImpl()
{
	if (pointer != nullptr)
	{
		Deleter()(static_cast<T *>(pointer));
		pointer = nullptr;
	}
}