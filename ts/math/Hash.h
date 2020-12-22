#pragma once

#include <type_traits>

TS_PACKAGE1(math)

extern uint32_t simpleHash32(const char *str, BigSizeType length);
extern uint32_t simpleHash32(const String &str);

extern uint64_t simpleHash64(const char *str, BigSizeType length);
extern uint64_t simpleHash64(const String &str);

template<class T>
uint32_t simpleHash32(const T &val)
{
	static_assert(std::is_trivially_copyable<T>::value, "Type must be trivially copyable. Specialization is required for complex types.");
	return simpleHash32(reinterpret_cast<const char *>(&val), sizeof(T));
}

template<class T>
uint32_t simpleHash32(const std::basic_string<T> &val)
{
	return simpleHash32(reinterpret_cast<const char *>(&val[0]), sizeof(T) * val.size());
}

template<class T>
uint64_t simpleHash64(const T &val)
{
	static_assert(std::is_trivially_copyable<T>::value, "Type must be trivially copyable. Specialization is required for complex types.");
	return simpleHash64(reinterpret_cast<const char *>(&val), sizeof(T));
}

template<class T>
uint64_t simpleHash64(const std::basic_string<T> &val)
{
	return simpleHash64(reinterpret_cast<const char *>(&val[0]), sizeof(T) * val.size());
}

TS_FORCEINLINE uint32_t hashCombine(uint32_t seed, uint32_t other)
{
	return seed ^ (other + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

template<class T>
uint32_t hashCombine(const uint32_t seed, const T &val)
{
	return hashCombine(seed, simpleHash32(val));
}

TS_FORCEINLINE uint64_t hashCombine(uint64_t seed, uint64_t other)
{
	return seed ^ (other + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2));
}

template<class T>
uint64_t hashCombine(const uint64_t seed, const T &val)
{
	return hashCombine(seed, simpleHash64(val));
}

// template<class T, class... Types>
// void hashCombine(uint64_t &seed, const T &val, const Types&... args)
// {
// 	seed ^= hashCombine(seed, simpleHash64(val));
// 	hashCombine(seed, args...);
// }
// 
// template<class... Types>
// uint64_t hashCombine(const Types&... args)
// {
// 	uint64_t seed = 0;
// 	hashCombine(seed, args...);
// 	return seed;
// }

TS_END_PACKAGE1()
