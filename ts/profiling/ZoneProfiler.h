#pragma once

#include "ts/lang/common/Common.h"

#include "ts/time/Time.h"
#include "ts/time/TimeSpan.h"

#include "ts/thread/RecursiveMutex.h"
#include "ts/thread/MutexGuard.h"

#include <vector>
#include <unordered_map>
#include <map>

#if TS_BUILD != TS_FINALRELEASE
	#define TS_PROFILER_ENABLED TS_FALSE
#else
	#define TS_PROFILER_ENABLED TS_FALSE
#endif

#if TS_PROFILER_ENABLED == TS_TRUE

#pragma warning( disable : 4456 ) // declaration hides previous local declaration

#define TS_ZONE()           const ts::profiling::ScopedZoneTimer __zoneTimer(TS_FUNCTION_LOG_SIMPLE ":" TS_EXPAND(__LINE__) )
#define TS_ZONE_NAMED(name) const ts::profiling::ScopedZoneTimer __zoneTimer(TS_FUNCTION_LOG_SIMPLE ":" TS_EXPAND(__LINE__) , name)

#define TS_ZONE_NAMED_VARIABLE(variable, name) const ts::profiling::ScopedZoneTimer variable(TS_FUNCTION_LOG_SIMPLE ":" TS_EXPAND(__LINE__), name)
#define TS_ZONE_MUTEX(owner, blocked)          const ts::profiling::ScopedZoneTimer __mutexZoneTimer(TS_FUNCTION_LOG_SIMPLE ":" TS_EXPAND(__LINE__), owner, blocked)

#define TS_ZONE_VARIABLE_FINISH(variable) (variable).commit()

#else

#define TS_ZONE()                              ((void)0)
#define TS_ZONE_NAMED(name)                    ((void)0)
#define TS_ZONE_NAMED_VARIABLE(variable, name) ((void)0)
#define TS_ZONE_MUTEX(owner, blocked)          ((void)0)

#define TS_ZONE_VARIABLE_FINISH(variable)      ((void)0)

#endif

TS_DECLARE_STRUCT2(engine, window, WindowView);

// Forward declare for SFML stuff
namespace sf {
class RenderTarget;
class Event;
}

TS_PACKAGE1(profiling)

struct ZoneEvent
{
	const char *name;
	int64 start;
	int64 elapsed;
	int32 level;
	uint32 mutexOwner;
	bool isMutexEvent;
	bool wasBlocked;
};

typedef std::vector<ZoneEvent> ZoneFrame;
typedef std::vector<ZoneFrame> ZoneFrameCollection;
typedef std::map<SizeType, ZoneFrameCollection> ZoneFrameCollectionStorage;

class ZoneProfiler
{
	friend class ScopedZoneTimer;
	friend class ZoneProfilerRenderer;

public:
	static ZoneProfiler &get();

	static void save(const String &filepath);
	static void commit(ZoneFrame &&stack);

	static void setEnabled(const bool enabled);
	static bool isEnabled();

	static void toggleVisibility();
	static bool isVisible();
	static bool handleEvent(const sf::Event &event);
	static void render(sf::RenderTarget &renderTarget, const engine::window::WindowView &view);

private:
	ZoneProfiler();
	~ZoneProfiler();

	static std::recursive_mutex mutex;

	static std::atomic_bool saving;
	static std::atomic_bool enabled;

	std::map<SizeType, std::string> threadNames;
	ZoneFrameCollectionStorage storage;
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
	static thread_local ZoneFrame currentFrame;
	static thread_local SizeType eventLevel;
};

TS_END_PACKAGE1()
