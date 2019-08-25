#pragma once

// #define __TS_VERIFY_POINTERS_IMPL_OLD(__return_value, ...) \
// 	do {\
// 		const void *__ptr[] = { ((void*)0), ## __VA_ARGS__ }; \
// 		const unsigned int __size = sizeof(__ptr) / sizeof(void *); \
// 		static_assert(__size > 1, "Pointer verification macro requires at least 1 pointer to check."); \
// 		for (unsigned int __i = 1; __i < __size; ++__i) { \
// 			if (__ptr[__i] == nullptr) return __return_value; \
// 		} \
// 	} while(false)

#define __TS_VERIFY_POINTERS_IMPL(__return_value, ...) \
	do { if (::ts::verify_pointers(__VA_ARGS__) == false) return __return_value; } while(false)

#define TS_VERIFY_POINTERS(...) __TS_VERIFY_POINTERS_IMPL(, __VA_ARGS__)
#define TS_VERIFY_POINTERS_WITH_RETURN_VALUE(__return_value, ...) __TS_VERIFY_POINTERS_IMPL(__return_value, __VA_ARGS__)

#include "ts/tessa/lang/ScopedPointer.h"
#include "ts/tessa/lang/SharedPointer.h"
#include "ts/tessa/lang/UniquePointer.h"

TS_PACKAGE0()

template<class PtrType, class... Args>
bool verify_pointers(PtrType &&ptr, Args&&... args)
{
	return ptr != nullptr && verify_pointers(std::forward<Args>(args)...);
}

template<class PtrType>
bool verify_pointers(PtrType &&ptr)
{
	return ptr != nullptr;
}

TS_END_PACKAGE0()
