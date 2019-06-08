#pragma once

#include <SFML/Config.hpp>

#pragma warning( disable : 4100 ) // Unreferenced function parameter
#pragma warning( disable : 4127 ) // Expression is constant
#pragma warning( disable : 4313 ) // Format string conflicts with argument type (seems to be bogus warning or something)

#define TS_VERSION_MAJOR 0
#define TS_VERSION_MINOR 1
#define TS_VERSION_PATCH 0

#define TS_WINDOWS	0x00000001
#define TS_LINUX	0x00000002

#if defined(WIN32) || defined(WIN64)
	#define TS_PLATFORM TS_WINDOWS
#elif defined(__LINUX__)
	#define TS_PLATFORM TS_LINUX
#else
	#error "Platform not supported."
#endif

#define TS_DEBUG        0x00000010
#define TS_RELEASE      0x00000020
#define TS_FINALRELEASE 0x00000030

#if defined(TS_BUILD_DEBUG)
	#define TS_BUILD TS_DEBUG
#elif defined(TS_BUILD_RELEASE)
	#define TS_BUILD TS_RELEASE
#elif defined(TS_BUILD_FINALRELEASE)
	#define TS_BUILD TS_FINALRELEASE
#else
	#error "Build type not defined."
#endif

#if TS_PLATFORM == TS_WINDOWS
	#define TS_NO_CONSOLE 1
#endif

#ifdef TS_NO_CONSOLE
	#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")	
#endif

#if defined(__GNUC__) || defined(__clang__)
	#define TS_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
	#define TS_PRETTY_FUNCTION __FUNCTION__
#else
	#error "Pretty function macro undefined on this compiler."
#endif

#define __TS_STRINGIFY(str) (# str)
#define TS_STRINGIFY(str)   __TS_STRINGIFY(str)

#include <cstdint>
#include <cinttypes>

TS_PACKAGE0()

typedef std::int8_t Int8;
typedef std::int16_t Int16;
typedef std::int32_t Int32;
typedef std::int64_t Int64;

typedef std::uint8_t Uint8;
typedef std::uint16_t Uint16;
typedef std::uint32_t Uint32;
typedef std::uint64_t Uint64;

typedef Uint32 SizeType;
typedef Uint64 BigSizeType;

// Template hack to get readable type names for classes
template<typename T>
struct TypeParseTraits;

#define TS_REGISTER_PARSE_TYPE(__type_name) \
	namespace ts { \
	template <> struct TypeParseTraits<__type_name> \
    { static const char* name; }; const char* TypeParseTraits<__type_name>::name = #__type_name; }

#define TS_GET_PARSE_TYPE(__type_name) TypeParseTraits<__type_name>::name

TS_END_PACKAGE0()
