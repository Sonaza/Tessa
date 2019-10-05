#pragma once

#define __TS_VERIFY_POINTERS_IMPL(__return_value, ...) \
	do { if (::ts::lang::verifyPointersImpl(__VA_ARGS__) == false) return __return_value; } while(false)

#define TS_VERIFY_POINTERS(...) __TS_VERIFY_POINTERS_IMPL(, __VA_ARGS__)
#define TS_VERIFY_POINTERS_WITH_RETURN_VALUE(__return_value, ...) __TS_VERIFY_POINTERS_IMPL(__return_value, __VA_ARGS__)

#include "ts/lang/ScopedPointer.h"
#include "ts/lang/SharedPointer.h"
#include "ts/lang/UniquePointer.h"

TS_PACKAGE1(lang)

template<class PtrType, class... Args>
bool verifyPointersImpl(PtrType &&ptr, Args&&... args)
{
	return ptr != nullptr && verifyPointersImpl(std::forward<Args>(args)...);
}

template<class PtrType>
bool verifyPointersImpl(PtrType &&ptr)
{
	return ptr != nullptr;
}

TS_END_PACKAGE1()
