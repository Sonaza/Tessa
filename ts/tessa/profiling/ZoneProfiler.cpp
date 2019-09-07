#include "Precompiled.h"
#include "ZoneProfiler.h"

#include "ts/tessa/file/OutputFile.h"
#include "ts/tessa/system/BaseApplication.h"
#include "ts/tessa/system/WindowViewManager.h"
#include "ts/tessa/thread/CurrentThread.h"
#include "ts/tessa/thread/Thread.h"
#include "ts/tessa/time/TimeSpan.h"
#include "ts/tessa/resource/ResourceManager.h"

#include <cinttypes>
#include <cmath>

#pragma warning( default : 4456 )

// #pragma optimize("", off)

TS_PACKAGE1(profiling)

namespace
{

void vertexArrayAppend(const sf::VertexArray &src, sf::VertexArray &dst)
{
	TS_ASSERT(dst.getVertexCount() == 0 || src.getPrimitiveType() == dst.getPrimitiveType());

	if (dst.getVertexCount() == 0)
		dst.setPrimitiveType(src.getPrimitiveType());

	BigSizeType startIndex = dst.getVertexCount();
	dst.resize(src.getVertexCount() + dst.getVertexCount());

	for (BigSizeType srcIndex = 0; srcIndex < src.getVertexCount(); ++srcIndex)
	{
		BigSizeType dstIndex = startIndex + srcIndex;
		dst[dstIndex] = src[srcIndex];
	}
}

}

class ZoneProfilerRenderer
{
public:
	static ZoneProfilerRenderer &get();

	void toggleVisibility()
	{
		visible = !visible;
	}

	bool isVisible() const
	{
		return visible;
	}

	math::VC2 lastMousePosition;

	bool handleEvent(const sf::Event &event)
	{
		switch (event.type)
		{
			case sf::Event::MouseMoved:
			{
				mousePosition = math::VC2((float)event.mouseMove.x, (float)event.mouseMove.y);
				math::VC2 delta = mousePosition - lastMousePosition;
				lastMousePosition = mousePosition;

				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					displayOffset += delta;
					displayOffset.x = math::max(0.f, displayOffset.x);
					displayOffset.y = math::max(0.f, displayOffset.y);
				}
			}
			break;

			case sf::Event::MouseWheelScrolled:
			{
				float delta = event.mouseWheelScroll.delta;
				timescale = math::clamp(timescale - delta * math::max(0.05f, (timescale / 30.f)), 0.01f, 500.f);

				displayOffset.x += delta * 50.f;
				displayOffset.x = math::max(0.f, displayOffset.x);
			}
			break;

			case sf::Event::KeyPressed:
			{
				switch (event.key.code)
				{
					case sf::Keyboard::F5:
					{
						
					}
					break;
				}
			}
			break;
		}

		return false;
	}

	float timescale = 35.f;
	math::VC2 displayOffset;

	void render(sf::RenderTarget &renderTarget, const system::WindowView &view)
	{
		std::unique_lock<std::recursive_mutex> lock(ZoneProfiler::mutex);

		{
			sf::RectangleShape shade(view.size);
			shade.setFillColor(sf::Color(0, 0, 0, 180));
			renderTarget.draw(shade);
		}

		static ZoneProfiler &profiler = ZoneProfiler::get();

		sf::Text threadText("", *debugFont, 16);
		threadText.setOutlineThickness(1.f);

		sf::VertexArray bars;

		static const float margin = 3.f;
		static const float height = 25.f;

		struct Label
		{
			math::FloatRect rect;
			String text;
		};
		std::vector<Label> labels;

		float offsetAccumulator = 0;
		PosType startTimeLimit = 0;

		EventStackCollectionStorage::const_iterator storageIter = profiler.storage.begin();
		for (; storageIter != profiler.storage.end(); ++storageIter)
		{
			const SizeType threadId = storageIter->first;
			if (threadId != 0)
				continue;

			threadText.setString(TS_FMT(
				"Thread #%u (%s)",
				threadId, profiler.threadNames[threadId]
			));

			const math::VC2 basePosition(view.size.x, 50.f + threadId * 220.f + height);

			threadText.setPosition(
				50.f,
				basePosition.y - height);

			renderTarget.draw(threadText);

			math::VC2 positionOffset(0.f, 0.f);

			const EventStackCollection &collection = storageIter->second;
// 			int64 firstStartTime = collection.back().front().start;

			if (threadId == 0)
			{
				EventStackCollection::const_reverse_iterator frameIter = collection.rbegin();
				for (; frameIter != collection.rend(); ++frameIter)
				{
					const EventList &events = *frameIter;
					float frameBaseWidth = events.front().elapsed / timescale;

					if (offsetAccumulator + frameBaseWidth >= displayOffset.x)
					{
						startTimeLimit = events.front().start;

						positionOffset.x = displayOffset.x - offsetAccumulator;

						break;
					}

					offsetAccumulator += frameBaseWidth;
				}
			}

			EventStackCollection::const_reverse_iterator frameIter = collection.rbegin();
			for (; frameIter != collection.rend(); ++frameIter)
			{
				const EventList &events = *frameIter;

				int64 startTime = events.front().start;
				if (startTime > startTimeLimit)
					continue;

				PosType frameIndex = std::distance(collection.begin(), frameIter.base());

				float frameBaseWidth = events.front().elapsed / timescale;
				positionOffset.x -= frameBaseWidth + margin;

// 				if (frameBaseWidth >= 6.f)
				{
					labels.push_back({
						math::FloatRect(basePosition + positionOffset, math::VC2(frameBaseWidth, 20.f), math::Rect_PositionAndSize),
						TS_FMT("%lld", frameIndex)
					});
				}

				for (EventList::const_iterator eventsIter = events.begin(); eventsIter != events.end(); ++eventsIter)
				{
					const EventFrame &frame = *eventsIter;

					float offsetX = (frame.level == 0 ? 0 : frame.start) / timescale;
					float frameWidth = math::max(1.f, frame.elapsed / timescale - margin);

					math::VC2 pos = math::VC2(basePosition + positionOffset)
						+ math::VC2(offsetX, (frame.level + 1.f) * (height + margin));

					math::VC2 size = math::VC2(frameWidth, height);

					math::FloatRect rect(pos, size, math::Rect_PositionAndSize);

					sf::Color color = frame.level < colors.size() ? colors[frame.level] : sf::Color::Magenta;
					if (rect.isPointWithin(mousePosition))
					{
						color += sf::Color(30, 30, 30);
					}

					addRectangle(
						bars,
						rect,
						color
					);

					if (frameWidth >= 10.f)
					{
						labels.push_back({
							rect,
							frame.name
						});
					}
				}

				if (math::abs(positionOffset.x) >= view.size.x)
					break;
			}

		}

		renderTarget.draw(bars);

		for (const Label &lbl : labels)
		{
			threadText.setString(lbl.text);
			threadText.setPosition(lbl.rect.minbounds);

			clipShader.setUniform("u_resolution", view.size);
			clipShader.setUniform("u_position", lbl.rect.minbounds);
			clipShader.setUniform("u_size", lbl.rect.getSize());

			renderTarget.draw(threadText, &clipShader);
		}
	}

	void addRectangle(sf::VertexArray &dst, const math::FloatRect &rect, const sf::Color &color)
	{
		if (dst.getVertexCount() == 0)
			dst.setPrimitiveType(sf::PrimitiveType::Quads);

		BigSizeType startIndex = dst.getVertexCount();
		dst.resize(dst.getVertexCount() + 4);

		const sf::Color topcolor(
			(uint8)math::min(255.f, (float)color.r + (color.r / 255.f) * 60.f),
			(uint8)math::min(255.f, (float)color.g + (color.g / 255.f) * 60.f),
			(uint8)math::min(255.f, (float)color.b + (color.b / 255.f) * 60.f),
			color.a
		);

		const math::VC2 position = rect.minbounds;
		const math::VC2 size = rect.getSize();

		dst[startIndex + 0] = sf::Vertex(position,                             topcolor);
		dst[startIndex + 1] = sf::Vertex(position + math::VC2(size.x, 0.f),    topcolor);
		dst[startIndex + 2] = sf::Vertex(position + math::VC2(size.x, size.y), color);
		dst[startIndex + 3] = sf::Vertex(position + math::VC2(0.f,    size.y), color);
	}

private:
	ZoneProfilerRenderer()
		: colors({
			sf::Color(174, 22, 22),
			sf::Color(214, 122, 14),
			sf::Color(232, 211, 34),
			sf::Color(102, 181, 18),
			sf::Color(30, 191, 147),
			sf::Color(53, 156, 243),
		})
	{
		system::BaseApplication &app = TS_GET_GIGATON().getGigaton<system::BaseApplication>();
		debugFont = &app.getDebugFont();

		String filepath = resource::ResourceManager::getAbsoluteResourcePath("shader/area_clip.frag");
		if (!clipShader.loadFromFile(filepath, sf::Shader::Fragment))
		{
			TS_ASSERT(!"Failed to load text clip shader.");
		}
	}

	const std::vector<sf::Color> colors;

	sf::Font *debugFont = nullptr;

	sf::Shader clipShader;

	math::VC2 mousePosition;

	bool visible = false;
};

ZoneProfilerRenderer &ZoneProfilerRenderer::get()
{
	static ZoneProfilerRenderer instance;
	return instance;
}

std::recursive_mutex ZoneProfiler::mutex;
std::atomic_bool ZoneProfiler::saving = false;

std::atomic_bool ZoneProfiler::enabled = true;

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

#define WRITE(format, ...) do { \
	const std::string str = TS_FMT("%s" format, std::string(indent, '\t'), ## __VA_ARGS__); \
	output.writeString(str); } while(0)
	// TS_PRINTF(str); \

void ZoneProfiler::save(const String &filepath)
{
	std::unique_lock<std::recursive_mutex> lock(mutex);

	ZoneProfiler &instance = get();

	if (instance.storage.empty())
	{
		TS_LOG_WARNING("Zone Profiler is has no profiling data to save.\n");
		saving = false;
		return;
	}

	std::map<SizeType, EventStackCollection> storage = instance.storage;

	lock.unlock();

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

			WRITE("<frame start=\"%lld\" elapsed=\"%lld\">\n", events.front().start, events.front().elapsed);
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

				if (frame.isMutexEvent)
				{
					if (frame.wasBlocked)
					{
						WRITE("<mutex start=\"%lld\" elapsed=\"%lld\" name=\"%s\" blockedBy=\"%u\" />\n",
							level == 0 ? 0 : frame.start,
							frame.elapsed,
							frame.name,
							frame.mutexOwner
						);
					}
					else
					{
						WRITE("<mutex start=\"%lld\" elapsed=\"%lld\" name=\"%s\" />\n",
							level == 0 ? 0 : frame.start,
							frame.elapsed,
							frame.name
						);
					}
				}
				else
				{
					WRITE("<event start=\"%lld\" elapsed=\"%lld\" name=\"%s\"%s>\n",
						level == 0 ? 0 : frame.start,
						frame.elapsed,
						frame.name,
						closedTag ? " /" : ""
					);
				}

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
}

#undef WRITE

void ZoneProfiler::commit(EventList &&events)
{
	static thread_local SizeType threadId = thread::CurrentThread::getThreadId();
	static thread_local std::string threadName = thread::CurrentThread::getThreadName();
	static thread_local bool threadNameSet = false;

	if (ZoneProfiler::enabled)
	{
		std::unique_lock<std::recursive_mutex> lock(mutex);

		ZoneProfiler &instance = get();

		TS_ASSERT(!events.empty());
		instance.storage[threadId].push_back(std::move(events));

		if (!threadNameSet)
		{
			instance.threadNames[threadId] = threadName;
			threadNameSet = true;
		}

		if (instance.storage[0].size() >= 100)
			ZoneProfiler::enabled = false;
	}
	else
	{
		events.clear();
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

void ZoneProfiler::toggleVisibility()
{
	ZoneProfilerRenderer::get().toggleVisibility();
}

bool ZoneProfiler::isVisible()
{
	return ZoneProfilerRenderer::get().isVisible();
}

bool ZoneProfiler::handleEvent(const sf::Event &event)
{
	return ZoneProfilerRenderer::get().handleEvent(event);
}

void ZoneProfiler::render(sf::RenderTarget &renderTarget, const system::WindowView &view)
{
	ZoneProfilerRenderer::get().render(renderTarget, view);
}

int64 ScopedZoneTimer::absoluteStartTime = -1;

thread_local int64 ScopedZoneTimer::frameStartTime = 0;
thread_local EventList ScopedZoneTimer::events;
thread_local SizeType ScopedZoneTimer::eventLevel = 0;

ScopedZoneTimer::ScopedZoneTimer(const char *functionName, const char *zoneName)
	: start(Time::now())
	, name(zoneName != nullptr ? zoneName : functionName)
{
	if (absoluteStartTime == -1)
		absoluteStartTime = Time::now().fromEpoch().getMicroseconds();

	const int64 startTime = start.fromEpoch().getMicroseconds();
	if (eventLevel == 0)
		frameStartTime = startTime;

	EventFrame ef = {
		name,
		eventLevel == 0 ? frameStartTime - absoluteStartTime : startTime - frameStartTime,
		-1,
		(int32)eventLevel,
		0, false, false
	};

	if (events.capacity() == 0)
		events.reserve(16);

	events.push_back(std::move(ef));
	frameIndex = events.size() - 1;
// 	frame = &events.back();

	eventLevel++;
}

ScopedZoneTimer::ScopedZoneTimer(const char *zoneName, uint32 mutexOwner, bool blocked)
	: start(Time::now())
	, name(zoneName)
{
	if (absoluteStartTime == -1)
		absoluteStartTime = Time::now().fromEpoch().getMicroseconds();

	const int64 startTime = start.fromEpoch().getMicroseconds();
	if (eventLevel == 0)
		frameStartTime = startTime;

	EventFrame ef = {
		name,
		eventLevel == 0 ? frameStartTime - absoluteStartTime : startTime - frameStartTime,
		-1,
		(int32)eventLevel,
		mutexOwner, true, blocked
	};

	events.push_back(std::move(ef));
	frameIndex = events.size() - 1;
// 	frame = &events.back();

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

	TimeSpan elapsed = Time::now() - start;

	TS_ASSERT(eventLevel > 0);
	eventLevel--;

// 	TS_ASSERT(frame != nullptr);
	EventFrame &frame = events[frameIndex];

	frame.elapsed = elapsed.getMicroseconds();
	TS_ASSERT(frame.elapsed >= 0);

	if (eventLevel == 0)
	{
		ZoneProfiler::commit(std::move(events));
		TS_ASSERT(events.empty());

	}
// 	else
// 	{
// 		EventList eventsCopy = events;
// 		ZoneProfiler::commit(std::move(eventsCopy));
// 
// 		events.erase(std::next(events.begin(), eventLevel), events.end());
// 	}
	
	committed = true;
}

TS_END_PACKAGE1()


