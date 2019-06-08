#pragma once

#define _TS_VERIFY_POINTERS_IMPL(__return_value, ...) \
	do {\
		const void *__ptr[] = { ((void*)0), ## __VA_ARGS__ }; \
		const unsigned int __size = sizeof(__ptr) / sizeof(void *); \
		static_assert(__size > 1, "Pointer verify macro requires at least 1 pointer to check."); \
		for (unsigned int __i = 1; __i < __size; ++__i) { \
			if (__ptr[__i] == nullptr) return __return_value; \
		} \
	} while(false)

#define TS_VERIFY_POINTERS(...) _TS_VERIFY_POINTERS_IMPL(, __VA_ARGS__)
#define TS_VERIFY_POINTERS_WITH_RETURN_VALUE(__return_value, ...) _TS_VERIFY_POINTERS_IMPL(__return_value, __VA_ARGS__)

TS_PACKAGE0()

template<class T, class... Args>
inline bool verify_pointers(T ptr, Args... args)
{
	return ptr != nullptr && verify_pointers(args...);
}

template<class T>
inline bool verify_pointers(T ptr)
{
	return ptr != nullptr;
}

TS_END_PACKAGE0()
