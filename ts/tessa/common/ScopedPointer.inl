
template <class T, class Deleter>
ScopedPointer<T, Deleter>::ScopedPointer(T *pointer)
{
	reset(pointer);
}

template <class T, class Deleter>
ScopedPointer<T, Deleter>::~ScopedPointer()
{
	reset();
}

template <class T, class Deleter>
T *ScopedPointer<T, Deleter>::get() const
{
	return pointer;
}

template <class T, class Deleter>
T *ScopedPointer<T, Deleter>::dismiss()
{
	T *ptr = pointer;
	pointer = nullptr;
	return ptr;
}

template <class T, class Deleter>
void ScopedPointer<T, Deleter>::reset(T *ptrParam)
{
	static_assert(std::is_array<T>::value == false, "ScopedPointer does not support storing arrays.");

	deletePointerDataIfNeeded();
	pointer = ptrParam;
}

template <class T, class Deleter>
typename std::add_lvalue_reference<T>::type ScopedPointer<T, Deleter>::operator*() const
{
	TS_ASSERTF(pointer != nullptr, "Attempting to dereference a null pointer");
	return *pointer;
}

template <class T, class Deleter>
T *ScopedPointer<T, Deleter>::operator->()
{
	TS_ASSERTF(pointer != nullptr, "Attempting indirection on a null pointer");
	return pointer;
}

template <class T, class Deleter>
ScopedPointer<T, Deleter>::operator bool()
{
	return pointer != nullptr;
}

template <class T, class Deleter>
ScopedPointer<T, Deleter>::operator void *()
{
	return static_cast<void*>(pointer);
}

template <class T, class Deleter>
bool ScopedPointer<T, Deleter>::operator!() const
{
	return pointer == nullptr;
}

template <class T, class Deleter>
bool ScopedPointer<T, Deleter>::operator==(nullptr_t) const
{
	return pointer == nullptr;
}

template <class T, class Deleter>
bool ScopedPointer<T, Deleter>::operator!=(nullptr_t) const
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
void ScopedPointer<T, Deleter>::deletePointerDataIfNeeded()
{
	if (pointer != nullptr)
	{
		struct DeleterImpl
		{
			static void destroy(T *pointer)
			{
				Deleter()(pointer);
			}
		};
		DeleterImpl::destroy(pointer);

		// 		delete pointer;
		// 		destructor(pointer);
		pointer = nullptr;
	}
}