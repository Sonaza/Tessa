
class lang::DefaultScopedPointerDeleter
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
ScopedPointer<T, Deleter>::ScopedPointer(T *pointer)
{
	reset(pointer);
}

template <class T, class Deleter>
ScopedPointer<T, Deleter>::ScopedPointer(std::nullptr_t)
{
}

template <class T, class Deleter>
ScopedPointer<T, Deleter>::~ScopedPointer()
{
	reset();
}

template <class T, class Deleter>
T *ScopedPointer<T, Deleter>::get() const
{
	return static_cast<T *>(pointer);
}

template <class T, class Deleter>
T *ScopedPointer<T, Deleter>::dismiss()
{
	T *ptr = static_cast<T *>(pointer);
	pointer = nullptr;
	return ptr;
}

template <class T, class Deleter>
void ScopedPointer<T, Deleter>::reset(T *ptrParam)
{
	_deleteImpl();
	pointer = ptrParam;
}

template <class T, class Deleter>
typename std::add_lvalue_reference<T>::type ScopedPointer<T, Deleter>::operator*() const
{
	TS_ASSERTF(pointer != nullptr, "Attempting to dereference a null pointer");
	return *static_cast<T *>(pointer);
}

template <class T, class Deleter>
T *ScopedPointer<T, Deleter>::operator->()
{
	TS_ASSERTF(pointer != nullptr, "Attempting indirection on a null pointer");
	return static_cast<T *>(pointer);
}

template <class T, class Deleter>
const T *ScopedPointer<T, Deleter>::operator->() const
{
	TS_ASSERTF(pointer != nullptr, "Attempting indirection on a null pointer");
	return static_cast<const T *>(pointer);
}

template <class T, class Deleter>
ScopedPointer<T, Deleter>::operator bool() const
{
	return pointer != nullptr;
}

template <class T, class Deleter>
ScopedPointer<T, Deleter>::operator void *() const
{
	return static_cast<void*>(pointer);
}

template <class T, class Deleter>
bool ScopedPointer<T, Deleter>::operator!() const
{
	return pointer == nullptr;
}

template <class T, class Deleter>
bool ScopedPointer<T, Deleter>::operator==(std::nullptr_t) const
{
	return pointer == nullptr;
}

template <class T, class Deleter>
bool ScopedPointer<T, Deleter>::operator!=(std::nullptr_t) const
{
	return pointer != nullptr;
}

template <class T, class Deleter>
bool ScopedPointer<T, Deleter>::operator==(const ScopedPointer &other) const
{
	return pointer == other.pointer;
}

template <class T, class Deleter>
bool ScopedPointer<T, Deleter>::operator!=(const ScopedPointer &other) const
{
	return pointer != other.pointer;
}

template <class T, class Deleter>
void ScopedPointer<T, Deleter>::swap(ScopedPointer &other)
{
	std::swap(pointer, other.pointer);
}

template <class T, class Deleter>
void ScopedPointer<T, Deleter>::_deleteImpl()
{
	if (pointer != nullptr)
	{
		Deleter()(static_cast<T *>(pointer));
		pointer = nullptr;
	}
}
