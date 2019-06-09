
template <class T>
ScopedPointer<T>::ScopedPointer(T *pointer)
{
	reset(pointer);
}

template <class T>
template <class Deleter>
ScopedPointer<T>::ScopedPointer(T *pointer, Deleter)
{
	reset(pointer, Deleter());
}

template <class T>
ScopedPointer<T>::~ScopedPointer()
{
	reset();
}

template <class T>
T *ScopedPointer<T>::get() const
{
	return pointer;
}

template <class T>
void ScopedPointer<T>::dismiss()
{
	pointer = nullptr;
	destructor = nullptr;
}

template <class T>
void ScopedPointer<T>::reset(T *ptrParam)
{
	reset(ptrParam, ScopedPointerDefaultDeleter<T>());
}

template <class T> template <class Deleter>
void ScopedPointer<T>::reset(T *ptrParam, Deleter)
{
	static_assert(std::is_array<T>::value == false, "ScopedPointer does not support storing arrays.");

	deletePointerDataIfNeeded();

	pointer = ptrParam;

	if (pointer != nullptr)
	{
		struct DeleterImpl
		{
			static void destroy(T *pointer)
			{
				Deleter()(pointer);
			}
		};
		destructor = &DeleterImpl::destroy;
	}
}

template <class T>
T &ScopedPointer<T>::operator*()
{
	TS_ASSERTF(pointer != nullptr, "Attempting to dereference a null pointer.");
	return *pointer;
}

template <class T>
T *ScopedPointer<T>::operator->()
{
	TS_ASSERTF(pointer != nullptr, "Attempting indirection on a null pointer.");
	return pointer;
}

template <class T>
ScopedPointer<T>::operator bool()
{
	return pointer != nullptr;
}

template <class T>
ScopedPointer<T>::operator void *()
{
	return static_cast<void*>(pointer);
}

template <class T>
bool ScopedPointer<T>::operator!() const
{
	return pointer == nullptr;
}

template <class T>
bool ScopedPointer<T>::operator==(nullptr_t) const
{
	return other.pointer == nullptr;
}

template <class T>
bool ScopedPointer<T>::operator!=(nullptr_t) const
{
	return other.pointer != nullptr;
}

template <class T>
bool ScopedPointer<T>::operator==(const ScopedPointer<T> &other) const
{
	return pointer == other.pointer;
}

template <class T>
bool ScopedPointer<T>::operator!=(const ScopedPointer<T> &other) const
{
	return pointer != other.pointer;
}

template <class T>
void ScopedPointer<T>::swap(ScopedPointer &other)
{
	std::swap(pointer, other.pointer);
	std::swap(destructor, other.destructor);
}

template <class T>
void ScopedPointer<T>::deletePointerDataIfNeeded()
{
	if (pointer != nullptr)
	{
// 		delete pointer;
		destructor(pointer);
		pointer = nullptr;
		destructor = nullptr;
	}
}