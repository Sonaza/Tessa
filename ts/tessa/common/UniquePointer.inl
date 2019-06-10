
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
		deletePointerDataIfNeeded();

		pointer = other.pointer;
		other.pointer = nullptr;
	}
	return *this;
}

template <class T, class Deleter>
T *UniquePointer<T, Deleter>::get() const
{
	return pointer;
}

template <class T, class Deleter>
T *UniquePointer<T, Deleter>::dismiss()
{
	T *ptr = pointer;
	pointer = nullptr;
	return ptr;
}

template <class T, class Deleter>
void UniquePointer<T, Deleter>::reset(T *ptrParam)
{
	static_assert(std::is_array<T>::value == false, "UniquePointer does not support storing arrays.");

	deletePointerDataIfNeeded();
	pointer = ptrParam;
}

template <class T, class Deleter>
typename std::add_lvalue_reference<T>::type UniquePointer<T, Deleter>::operator*() const
{
	TS_ASSERTF(pointer != nullptr, "Attempting to dereference a null pointer");
	return *pointer;
}

template <class T, class Deleter>
T *UniquePointer<T, Deleter>::operator->()
{
	TS_ASSERTF(pointer != nullptr, "Attempting indirection on a null pointer");
	return pointer;
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
void UniquePointer<T, Deleter>::deletePointerDataIfNeeded()
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