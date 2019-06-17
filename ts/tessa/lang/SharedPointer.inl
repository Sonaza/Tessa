
class lang::DefaultSharedPointerDeleter
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

template <class T>
SharedPointer<T>::SharedPointer(T *ptr)
{
	reset(ptr);
}

template <class T>
template <class T2>
SharedPointer<T>::SharedPointer(const SharedPointer<T2> &other, T *ptr)
{
	pointer = ptr;
	impl = (SharedPointer<T>::SharedPointerImpl*)other.impl;
	increaseReferenceCounter();
}

template <class T>
template <class Deleter>
SharedPointer<T>::SharedPointer(T *ptr, Deleter)
{
	reset(ptr, Deleter());
}

template <class T>
SharedPointer<T>::~SharedPointer()
{
	reset();
}

template <class T>
SharedPointer<T>::SharedPointer(const SharedPointer<T> &other)
{
	*this = other;
}

template <class T>
SharedPointer<T> &SharedPointer<T>::operator=(const SharedPointer<T> &other)
{
	// Prevent self assignment
	if (this != &other)
	{
		decreaseReferenceCounter();
		pointer = other.pointer;
		impl = other.impl;
		increaseReferenceCounter();
	}
	return *this;
}

template <class T>
SharedPointer<T>::SharedPointer(SharedPointer<T> &&other)
{
	*this = other;
}

template <class T>
SharedPointer<T> &SharedPointer<T>::operator=(SharedPointer<T> &&other)
{
	// Prevent self assignment
	if (this != &other)
	{
		decreaseReferenceCounter();
		pointer = other.pointer;
		impl = other.impl;

		other.pointer = nullptr;
		other.impl = nullptr;
	}
	return *this;
}

template <class T>
T *SharedPointer<T>::get() const
{
	return pointer;
}

template <class T>
void SharedPointer<T>::reset(T *ptrParam)
{
	reset(ptrParam, lang::DefaultSharedPointerDeleter());
}

template <class T> template <class Deleter>
void SharedPointer<T>::reset(T *ptrParam, Deleter)
{
	decreaseReferenceCounter();

	pointer = ptrParam;
	impl = nullptr;
	if (pointer != nullptr)
	{
		impl = new SharedPointerImpl;
		
		struct DeleterImpl
		{
			static void destroy(void *pointer)
			{
				Deleter()(static_cast<T *>(pointer));
			}
		};
		impl->destructor = &DeleterImpl::destroy;

		impl->refcount.store(1, std::memory_order_release);
	}
}

template <class T>
typename std::add_lvalue_reference<T>::type SharedPointer<T>::operator*() const
{
	TS_ASSERTF(pointer != nullptr, "Attempting to dereference a null pointer");
	return *pointer;
}

template <class T>
T *SharedPointer<T>::operator->() const
{
	TS_ASSERTF(pointer != nullptr, "Attempting indirection on a null pointer");
	return pointer;
}

template <class T>
SharedPointer<T>::operator bool()
{
	return pointer != nullptr;
}

template <class T>
SharedPointer<T>::operator void *()
{
	return static_cast<void*>(pointer);
}

template <class T>
bool SharedPointer<T>::operator!() const
{
	return pointer == nullptr;
}

template <class T>
bool SharedPointer<T>::operator==(nullptr_t) const
{
	return pointer == nullptr;
}

template <class T>
bool SharedPointer<T>::operator!=(nullptr_t) const
{
	return pointer != nullptr;
}

template <class T>
bool SharedPointer<T>::operator==(const SharedPointer<T> &other) const
{
	return pointer == other.pointer;
}

template <class T>
bool SharedPointer<T>::operator!=(const SharedPointer<T> &other) const
{
	return pointer != other.pointer;
}

template <class T>
template <class T2>
bool SharedPointer<T>::operator==(const SharedPointer<T2> &other) const
{
	return pointer == other.pointer;
}

template <class T>
template <class T2>
bool SharedPointer<T>::operator!=(const SharedPointer<T2> &other) const
{
	return pointer != other.pointer;
}

template <class T>
bool SharedPointer<T>::isUnique() const
{
	TS_ASSERTF(pointer != nullptr, "A null pointer has no references");
	return impl->refcount.load(std::memory_order_relaxed) == 1;
}

template <class T>
void SharedPointer<T>::swap(SharedPointer &other)
{
	std::swap(pointer, other.pointer);
	std::swap(impl, other.impl);
}

template <class T>
void SharedPointer<T>::increaseReferenceCounter()
{
	if (impl != nullptr)
	{
		TS_ASSERT(pointer != nullptr);
		impl->refcount.fetch_add(1, std::memory_order_relaxed);
	}
	else
	{
		TS_ASSERT(pointer == nullptr);
	}
}

template <class T>
void SharedPointer<T>::decreaseReferenceCounter()
{
	if (impl != nullptr)
	{
		const Int32 oldRefCount = impl->refcount.fetch_sub(1, std::memory_order_release);

		// Reached zero references
		if (oldRefCount == 1)
		{
			std::atomic_thread_fence(std::memory_order_acquire);

			TS_ASSERT(pointer != nullptr);

// 			delete pointer;
			impl->destructor(pointer);
			pointer = nullptr;
				
			delete impl;
			impl = nullptr;
		}
	}
}
