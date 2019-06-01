#pragma once

#define TS_VERIFY_POINTERS(...) do { if(!::ts::verify_pointers(__VA_ARGS__)) return; } while(false)
#define TS_VERIFY_POINTERS_WITH_RETURN_VALUE(__return_value, ...) do { if(!::ts::verify_pointers(__VA_ARGS__)) return __return_value; } while(false)

TS_PACKAGE0()

template<class T, class... Args>
bool verify_pointers(T ptr, Args... args)
{
	return ptr != nullptr && verify_pointers(args...);
}

template<class T>
bool verify_pointers(T ptr)
{
	return ptr != nullptr;
}

TS_END_PACKAGE0()
