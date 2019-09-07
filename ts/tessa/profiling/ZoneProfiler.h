#pragma once

#include "ts/tessa/common/Common.h"

#include "ts/tessa/time/Time.h"
#include "ts/tessa/time/TimeSpan.h"

#include "ts/tessa/thread/RecursiveMutex.h"
#include "ts/tessa/thread/MutexGuard.h"

#include <vector>
#include <unordered_map>

#if TS_BUILD != TS_FINALRELEASE
	#define TS_PROFILER_ENABLED TS_TRUE
#else
	#define TS_PROFILER_ENABLED TS_FALSE
#endif

#if TS_PROFILER_ENABLED == TS_TRUE

#pragma warning( disable : 4456 ) // declaration hides previous local declaration

#define TS_ZONE()           const ts::profiling::ScopedZoneTimer __zoneTimer(TS_FUNCTION_LOG_SIMPLE ":" TS_EXPAND(__LINE__) )
#define TS_ZONE_NAMED(name) const ts::profiling::ScopedZoneTimer __zoneTimer(TS_FUNCTION_LOG_SIMPLE ":" TS_EXPAND(__LINE__) , name)

#define TS_ZONE_NAMED_VARIABLE(variable, name) const ts::profiling::ScopedZoneTimer variable(TS_FUNCTION_LOG_SIMPLE ":" TS_EXPAND(__LINE__), name)
#define TS_ZONE_MUTEX(owner, blocked)          const ts::profiling::ScopedZoneTimer __mutexZoneTimer(TS_FUNCTION_LOG_SIMPLE ":" TS_EXPAND(__LINE__), owner, blocked)

#else

#define TS_ZONE()                              ((void)0)
#define TS_ZONE_NAMED(name)                    ((void)0)
#define TS_ZONE_NAMED_VARIABLE(variable, name) ((void)0)
#define TS_ZONE_MUTEX(owner, blocked)          ((void)0)

#endif

TS_DECLARE_STRUCT1(system, WindowView);

TS_PACKAGE1(profiling)

struct EventFrame
{
	const char *name;
	int64 start;
	int64 elapsed;
	int32 level;
	uint32 mutexOwner;
	bool isMutexEvent;
	bool wasBlocked;
};

typedef std::vector<EventFrame> EventList;
typedef std::vector<EventList> EventStackCollection;
typedef std::map<SizeType, EventStackCollection> EventStackCollectionStorage;

class ZoneProfiler
{
	friend class ScopedZoneTimer;
	friend class ZoneProfilerRenderer;

public:
	static ZoneProfiler &get();

	static void save(const String &filepath);
	static void commit(EventList &&stack);

	static void setEnabled(const bool enabled);
	static bool isEnabled();

	static void toggleVisibility();
	static bool isVisible();
	static bool handleEvent(const sf::Event &event);
	static void render(sf::RenderTarget &renderTarget, const system::WindowView &view);

private:
	ZoneProfiler();
	~ZoneProfiler();

	static std::recursive_mutex mutex;

	static std::atomic_bool saving;
	static std::atomic_bool enabled;

	std::map<SizeType, std::string> threadNames;
	EventStackCollectionStorage storage;
};

class ScopedZoneTimer
{
	friend class ZoneProfiler;

public:
	ScopedZoneTimer(const char *functionName, const char *zoneName = nullptr);
	ScopedZoneTimer(const char *zoneName, uint32 mutexOwner, bool blocked);
	~ScopedZoneTimer();

	void commit() const;

private:
	mutable std::atomic_bool committed = false;

	Time start;
	const char *name = nullptr;
// 	EventFrame *frame = nullptr;

	BigSizeType frameIndex = 0;
	
	static int64 absoluteStartTime;

	static thread_local int64 frameStartTime;
	static thread_local EventList events;
	static thread_local SizeType eventLevel;
};

TS_END_PACKAGE1()
