#pragma once

#include "ts/tessa/common/Common.h"

#include "ts/tessa/time/Time.h"
#include "ts/tessa/time/TimeSpan.h"

#include "ts/tessa/thread/Mutex.h"
#include "ts/tessa/thread/MutexGuard.h"

#include <vector>
#include <unordered_map>

// #define TS_PROFILER_ENABLED TS_TRUE

#if TS_BUILD != TS_FINALRELEASE

#pragma warning( disable : 4456 ) // declaration hides previous local declaration

#define TS_ZONE()           const ts::profiling::ScopedZoneTimer __simpleZoneTimer(TS_FUNCTION_LOG_SIMPLE ":" TS_EXPAND(__LINE__) )
#define TS_ZONE_NAMED(name) const ts::profiling::ScopedZoneTimer __simpleZoneTimer(TS_FUNCTION_LOG_SIMPLE ":" TS_EXPAND(__LINE__) , name)

#define TS_ZONE_NAMED_VARIABLE(variable, name) const ts::profiling::ScopedZoneTimer variable(TS_FUNCTION_LOG_SIMPLE ":" TS_EXPAND(__LINE__) , name)

#else

#define TS_ZONE()           ((void)0)
#define TS_ZONE_NAMED(name) ((void)0)

#endif

TS_PACKAGE1(profiling)

struct EventFrame
{
	const char *name;
	int64 start;
	int64 elapsed;
	int32 level;
};

typedef std::list<EventFrame> EventList;
typedef std::vector<EventList> EventStackCollection;

class ZoneProfiler
{
	friend class ScopedZoneTimer;

public:
	static ZoneProfiler &get();

	static void save(const String &filepath);
	static void commit(EventList &&stack);

	static void setEnabled(const bool enabled);
	static bool isEnabled();

private:
	ZoneProfiler();
	~ZoneProfiler();

	static Mutex mutex;

	static std::atomic_bool saving;

	static std::atomic_bool enabled;
	static std::atomic_bool enabledAccepting;

	std::map<SizeType, std::string> threadNames;
	std::map<SizeType, EventStackCollection> storage;
};

class ScopedZoneTimer
{
	friend class ZoneProfiler;

public:
	ScopedZoneTimer(const char *functionName, const char *zoneName = nullptr);
	~ScopedZoneTimer();

	void commit() const;

private:
	mutable std::atomic_bool committed = false;

	Time start;
	const char *name = nullptr;
	EventFrame *frame = nullptr;
	
	static thread_local int64 frameStartTime;
	static thread_local EventList events;
	static thread_local SizeType eventLevel;
};

TS_END_PACKAGE1()
