#pragma once

#pragma warning( disable : 4100 ) // unreferenced function parameter
#pragma warning( disable : 4127 ) // expression is constant
#pragma warning( disable : 4201 ) // nonstandard extension used: nameless struct/union
#pragma warning( disable : 4313 ) // format string conflicts with argument type (seems to be bogus warning or something)

#pragma warning( disable : 4189 ) // local variable is initialized but not referenced

#define TS_VERSION_MAJOR 0
#define TS_VERSION_MINOR 1
#define TS_VERSION_PATCH 0

#define TS_TRUE  1
#define TS_FALSE 0

#define TS_BIG_ENDIAN 0x01
#define TS_LITTLE_ENDIAN 0x02
#define TS_BYTE_ORDER TS_LITTLE_ENDIAN

///////////////////////////////////////////////////////
// Platform

#define TS_WINDOWS	0x01
#define TS_LINUX	0x02

#if defined(_WIN32)
	#define TS_PLATFORM TS_WINDOWS
#elif defined(__linux__)
	#define TS_PLATFORM TS_LINUX
#else
	#error "Platform not supported."
#endif

///////////////////////////////////////////////////////
// Compilers

#define TS_MSC    0x01
#define TS_GNUC   0x02
#define TS_CLANG  0x03

#if defined(_MSC_VER)
	#define TS_COMPILER TS_MSC
#elif defined(__GNUC__)
	#define TS_COMPILER TS_GNUC
#elif defined(__clang__)
	#define TS_COMPILER TS_CLANG
#else
	#error "Compiler not supported"
#endif

///////////////////////////////////////////////////////
// Build mode

#define TS_DEBUG        0x01
#define TS_RELEASE      0x02
#define TS_FINALRELEASE 0x03

#if defined(TS_BUILD_DEBUG)
	#define TS_BUILD TS_DEBUG
#elif defined(TS_BUILD_RELEASE)
	#define TS_BUILD TS_RELEASE
#elif defined(TS_BUILD_FINALRELEASE)
	#define TS_BUILD TS_FINALRELEASE
#else
	#error "Build type not defined."
#endif

///////////////////////////////////////////////////////
// Useful macros

#if TS_COMPILER == TS_MSC
	#define TS_FUNCTION_LOG_SIMPLE __FUNCTION__
	#define TS_FUNCTION_LOG_DETAIL __FUNCSIG__
#elif TS_COMPILER == TS_GNUC || TS_COMPILER == TS_CLANG
	#define TS_FUNCTION_LOG_SIMPLE __PRETTY_FUNCTION__
	#define TS_FUNCTION_LOG_DETAIL __PRETTY_FUNCTION__
#else
	#define TS_FUNCTION_LOG_SIMPLE __func__
	#define TS_FUNCTION_LOG_DETAIL __func__
#endif

#if TS_COMPILER == TS_MSC
	#define TS_FORCEINLINE __forceinline
	#define TS_ALIGN(alignment) __declspec(align(alignment))
#elif TS_COMPILER == TS_GNUC || TS_COMPILER == TS_CLANG
	#define TS_FORCEINLINE inline __attribute__((always_inline))
	#define TS_ALIGN(alignment) __attribute__((aligned(alignment)))
#else
	#define TS_FORCEINLINE inline
	#define TS_ALIGN(alignment) alignas(alignment)
#endif

#if TS_COMPILER == TS_MSC
	#define TS_RESTRICT __restrict
#elif TS_COMPILER == TS_GNUC || TS_COMPILER == TS_CLANG
	#define TS_RESTRICT __restrict__
#else
	#define TS_RESTRICT __restrict
#endif

#define TS_EXPAND_1(x) #x
#define TS_EXPAND(x)   TS_EXPAND_1(x)

#define TS_UNUSED_VARIABLE(variable) ((void)variable)

#define TS_ARRAY_SIZE(array) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#include <cstdint>
#include <cinttypes>

#include "ts/lang/common/Package.h"

TS_PACKAGE0()

using std::int8_t;
using std::uint8_t;

using std::int16_t;
using std::uint16_t;

using std::int32_t;
using std::uint32_t;

using std::int64_t;
using std::uint64_t;

/*
using int8_t    = std::int8_t;
using uint8_t   = std::uint8_t;

using int16_t   = std::int16_t;
using uint16_t  = std::uint16_t;

using int32_t   = std::int32_t;
using uint32_t  = std::uint32_t;

using int64_t   = std::int64_t;
using uint64_t  = std::uint64_t;
*/

/*
typedef signed   char  int8_t;
typedef unsigned char  uint8_t;

typedef signed   short int16_t;
typedef unsigned short uint16_t;

typedef signed   int   int32_t;
typedef unsigned int   uint32_t;

#if TS_COMPILER == TS_MSC
	typedef signed   __int64 int64_t;
	typedef unsigned __int64 uint64_t;
#else
	typedef signed   long long int64_t;
	typedef unsigned long long uint64_t;
#endif
*/

typedef uint8_t Byte;

typedef uint32_t SizeType;
typedef uint64_t BigSizeType;

typedef signed   long long int64sfml_t;
typedef int64sfml_t PosType;

// Template hack to get readable type names for classes
template<typename T>
struct TypeParseTraits;

#define TS_REGISTER_PARSE_TYPE(__type_name) \
	namespace ts { \
	template <> struct TypeParseTraits<__type_name> \
    { static const char* name; }; const char* TypeParseTraits<__type_name>::name = #__type_name; }

#define TS_GET_PARSE_TYPE(__type_name) ::ts::TypeParseTraits<__type_name>::name

TS_END_PACKAGE0()
