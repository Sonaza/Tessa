
template <class T>
UniquePointer<T>::UniquePointer(T *pointer)
{
	reset(pointer);
}

template <class T>
template <class Deleter>
UniquePointer<T>::UniquePointer(T *pointer, Deleter)
{
	reset(pointer, Deleter());
}

template <class T>
UniquePointer<T>::~UniquePointer()
{
	reset();
}

template <class T>
UniquePointer<T>::UniquePointer(UniquePointer<T> &&other)
{
	*this = std::move(other);
}

template <class T>
UniquePointer<T> &UniquePointer<T>::operator=(UniquePointer<T> &&other)
{
	// Prevent self assignment
	if (this != &other)
	{
		deletePointerDataIfNeeded();

		pointer = other.pointer;
		destructor = other.destructor;

		other.pointer = nullptr;
		other.destructor = nullptr;
	}
	return *this;
}

template <class T>
T *UniquePointer<T>::get() const
{
	return pointer;
}

template <class T>
void UniquePointer<T>::dismiss()
{
	pointer = nullptr;
	destructor = nullptr;
}

template <class T>
void UniquePointer<T>::reset(T *ptrParam)
{
	reset(ptrParam, UniquePointerDefaultDeleter<T>());
}

template <class T> template <class Deleter>
void UniquePointer<T>::reset(T *ptrParam, Deleter)
{
	static_assert(std::is_array<T>::value == false, "UniquePointer does not support storing arrays.");

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
T &UniquePointer<T>::operator*()
{
	TS_ASSERTF(pointer != nullptr, "Attempting to dereference a null pointer.");
	return *pointer;
}

template <class T>
T *UniquePointer<T>::operator->()
{
	TS_ASSERTF(pointer != nullptr, "Attempting indirection on a null pointer.");
	return pointer;
}

template <class T>
UniquePointer<T>::operator bool()
{
	return pointer != nullptr;
}

template <class T>
UniquePointer<T>::operator void *()
{
	return static_cast<void*>(pointer);
}

template <class T>
bool UniquePointer<T>::operator!() const
{
	return pointer == nullptr;
}

template <class T>
bool UniquePointer<T>::operator==(nullptr_t) const
{
	return other.pointer == nullptr;
}

template <class T>
bool UniquePointer<T>::operator!=(nullptr_t) const
{
	return other.pointer != nullptr;
}

template <class T>
bool UniquePointer<T>::operator==(const UniquePointer<T> &other) const
{
	return pointer == other.pointer;
}

template <class T>
bool UniquePointer<T>::operator!=(const UniquePointer<T> &other) const
{
	return pointer != other.pointer;
}

template <class T>
void UniquePointer<T>::swap(UniquePointer &other)
{
	std::swap(pointer, other.pointer);
	std::swap(destructor, other.destructor);
}

template <class T>
void UniquePointer<T>::deletePointerDataIfNeeded()
{
	if (pointer != nullptr)
	{
// 		delete pointer;
		destructor(pointer);
		pointer = nullptr;
		destructor = nullptr;
	}
}