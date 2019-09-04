#include "Precompiled.h"
#include "ScopedZoneTimer.h"

#include "ts/tessa/thread/Thread.h"
#include "ts/tessa/thread/CurrentThread.h"

#include "ts/tessa/file/OutputFile.h"

#include "ts/tessa/time/TimeSpan.h"
#include <cinttypes>

#pragma optimize("", off)

TS_PACKAGE1(profiling)

Mutex ZoneProfiler::mutex;
std::atomic_bool ZoneProfiler::saving = false;

std::atomic_bool ZoneProfiler::enabled = true;
std::atomic_bool ZoneProfiler::enabledAccepting = true;

ZoneProfiler::ZoneProfiler()
{
	
}

ZoneProfiler::~ZoneProfiler()
{

}

ZoneProfiler &ZoneProfiler::get()
{
	static ZoneProfiler instance;
	return instance;
}

#define WRITE(format, ...) do { output.writeString(TS_FMT("%s" format, std::string(indent, '\t'), ## __VA_ARGS__)); } while(0)

void ZoneProfiler::save(const String &filepath)
{
// 	MutexGuard lock(mutex);
	saving = true;

	ZoneProfiler &instance = get();
	std::map<SizeType, EventStackCollection> storage = std::move(instance.storage);

	saving = false;

	int32 indent = 0;

	file::OutputFile output(filepath, file::OutputFileMode_WriteBinaryTruncate);

	output.writeString("<root>\n");
	indent++;

	int64 numThreads = storage.size();
	int64 numFrames = 0;
	int64 numEvents = 0;

	for (auto storageIter = storage.begin(); storageIter != storage.end(); ++storageIter)
	{
		const SizeType threadId = storageIter->first;
		const std::string &threadName = instance.threadNames[threadId];

		WRITE("<thread id=\"%u\" name=\"%s\">\n", threadId, threadName);
		indent++;

		EventStackCollection &collection = storageIter->second;
		
		numFrames += collection.size();

		for (SizeType frameIndex = 0; frameIndex < collection.size(); ++frameIndex)
		{
			const EventList &events = collection[frameIndex];

			WRITE("<frame start=\"%lld\">\n", events.front().start);
			indent++;

			numEvents += events.size();

			int32 openings = 0;
			for (auto eventsIter = events.begin(); eventsIter != events.end(); ++eventsIter)
			{
				const EventFrame &frame = *eventsIter;

				int32 level = frame.level;

				auto nextIter = std::next(eventsIter);
				int32 nextLevel = nextIter == events.end() ? 0 : nextIter->level;

				bool closedTag = (nextLevel <= level);

				WRITE("<event start=\"%lld\" elapsed=\"%lld\" name=\"%s\"%s>\n",
					level == 0 ? 0 : frame.start,
					frame.elapsed,
					frame.name,
					closedTag ? " /" : ""
				);

				if (!closedTag)
				{
					openings++;
					indent++;
				}
				else if (nextLevel < level)
				{
					int32 diff = (level - nextLevel);
					for (int32 i = 0; i < diff; ++i)
					{
						indent--;
						WRITE("</event>\n");
						openings--;
					}
				}

				TS_ASSERT(indent >= 0);
			}
			TS_ASSERT(openings == 0);

			indent--;
			WRITE("</frame>\n");

			TS_ASSERT(indent >= 0);
		}

		indent--;
		WRITE("</thread>\n");

		TS_ASSERT(indent >= 0);
	}

	indent--;
	output.writeString("</root>\n");

	output.close();

	TS_PRINTF("Zone Profiler colleced data written to '%s'\n", filepath);
	TS_PRINTF("  Number of Threads          : %lld\n", numThreads);
	TS_PRINTF("  Number of Collected Frames : %lld\n", numFrames);
	TS_PRINTF("  Number of Collected Events : %lld\n", numEvents);
	TS_PRINTF("\n");

// 	saving = false;
}

#undef WRITE_INDENT
#undef WRITE

void ZoneProfiler::commit(EventList &&stack)
{
	static thread_local SizeType threadId = thread::CurrentThread::getThreadId();
	static thread_local std::string threadName = thread::CurrentThread::getThreadName();
	static thread_local bool threadNameSet = false;

	while (saving)
		Thread::sleep(10_ms);

	ZoneProfiler &instance = get();

	instance.storage[threadId].push_back(std::move(stack));

	if (!threadNameSet)
	{
		instance.threadNames[threadId] = threadName;
		threadNameSet = true;
	}
}

void ZoneProfiler::setEnabled(const bool enabledParam)
{
	enabled = enabledParam;
}

bool ZoneProfiler::isEnabled()
{
	return enabled.load();
}

thread_local int64 ScopedZoneTimer::frameStartTime = 0;
thread_local EventList ScopedZoneTimer::events;
thread_local SizeType ScopedZoneTimer::eventLevel = 0;

ScopedZoneTimer::ScopedZoneTimer(const char *functionName, const char *zoneName)
	: start(Time::now())
	, name(zoneName != nullptr ? zoneName : functionName)
{
	if (ZoneProfiler::enabled == true && ZoneProfiler::enabledAccepting == false && eventLevel == 0)
	{
		ZoneProfiler::enabledAccepting = true;
	}

	if (ZoneProfiler::enabledAccepting)
	{
		static int64 absoluteStartTime = start.fromEpoch().getMicroseconds();

		const int64 startTime = start.fromEpoch().getMicroseconds();
		if (eventLevel == 0)
			frameStartTime = startTime;

		EventFrame ef = {
			name,
			eventLevel == 0 ? frameStartTime - absoluteStartTime : startTime - frameStartTime,
			(int32)eventLevel,
			-1
		};

		events.push_back(std::move(ef));
		frame = &events.back();
	}

	eventLevel++;
}

ScopedZoneTimer::~ScopedZoneTimer()
{
	commit();
}

void ScopedZoneTimer::commit() const
{
	if (committed)
		return;

	TS_ASSERT(eventLevel > 0);
	eventLevel--;

	if (ZoneProfiler::enabled == false && ZoneProfiler::enabledAccepting == true && eventLevel == 0)
	{
		ZoneProfiler::enabledAccepting = false;
	}

	if (ZoneProfiler::enabledAccepting == false)
		return;

	TS_ASSERT(frame != nullptr);

	TimeSpan elapsed = Time::now() - start;
	frame->elapsed = elapsed.getMicroseconds();

	if (eventLevel == 0)
	{
		ZoneProfiler::commit(std::move(events));
		TS_ASSERT(events.empty());

		committed = true;
	}
	else
	{
		EventList eventsCopy = events;
		ZoneProfiler::commit(std::move(eventsCopy));

		events.erase(std::next(events.begin(), eventLevel), events.end());
	}
}

TS_END_PACKAGE1()


