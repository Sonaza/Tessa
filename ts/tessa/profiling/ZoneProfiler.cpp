#include "Precompiled.h"
#include "ZoneProfiler.h"

#include "ts/tessa/file/OutputFile.h"
#include "ts/tessa/system/BaseApplication.h"
#include "ts/tessa/system/WindowManager.h"
#include "ts/tessa/system/WindowViewManager.h"
#include "ts/tessa/thread/CurrentThread.h"
#include "ts/tessa/thread/Thread.h"
#include "ts/tessa/time/TimeSpan.h"
#include "ts/tessa/resource/ResourceManager.h"
#include "ts/tessa/resource/ShaderResource.h"

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
		const system::WindowView &view = windowManager->getInterfaceView();

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
					displayOffset.x = math::clamp(displayOffset.x, 0.f, baseStartTime / timescale - view.size.x);
					displayOffset.y = math::min(0.f, displayOffset.y);
				}
			}
			break;

			case sf::Event::MouseWheelScrolled:
			{
				float delta = event.mouseWheelScroll.delta;

				float lastTimeScale = timescale;
				timescale = math::clamp(timescale - delta * math::max(0.1f, (timescale / 20.f)), 0.01f, 1500.f);
				
				float diff = lastTimeScale - timescale;

				TS_PRINTF("Time scale  %0.2f\n", timescale);
				TS_PRINTF("Scale diff  %0.2f\n", diff);

// 				displayOffset.x += delta * diff * view.size.x * 0.5f;
// 				displayOffset.x = math::clamp(displayOffset.x, 0.f, baseStartTime / timescale);
// 				TS_PRINTF("Offset      %0.2f\n", displayOffset.x);
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

	float timescale = 8.0f;
	math::VC2 displayOffset;

	int64 baseStartTime = 0;

	void render(sf::RenderTarget &renderTarget, const system::WindowView &view)
	{
		std::unique_lock<std::recursive_mutex> lock(ZoneProfiler::mutex);

		{
			sf::RectangleShape shade(view.size);
			shade.setFillColor(sf::Color(0, 0, 0, 180));
			renderTarget.draw(shade);
		}

		static ZoneProfiler &profiler = ZoneProfiler::get();

		// Skedaddle if main thread doesn't have data
		if (profiler.storage.count(0) == 0)
			return;

		sf::Text threadText("", *debugFont, 15);
		threadText.setOutlineThickness(1.f);

		sf::VertexArray bars;

		static const float margin = 3.f;
		static const float height = 20.f;

		struct Label
		{
			math::FloatRect rect;
			String text;
		};
		std::vector<Label> labels;

		float offsetAccumulator = 0;
		int64 startTimeLimit = 0;
		float timeDisplayOffset = 0.f;

		baseStartTime = profiler.storage[0].back().front().start;

		struct HoverInfo
		{
			HoverInfo() = default;
			HoverInfo(SizeType threadId, String threadName, int64 frameIndex, const ZoneFrame *frame, const ZoneEvent *event)
				: threadId(threadId)
				, threadName(threadName)
				, frameIndex(frameIndex)
				, frame(frame)
				, event(event)
			{
			}

			SizeType threadId = 0;
			String threadName;
			int64 frameIndex = 0;
			const ZoneFrame *frame = nullptr;
			const ZoneEvent *event = nullptr;

			bool isValid() const { return frame != nullptr && event != nullptr; }
		};
		HoverInfo hoverInfo;

		ZoneFrameCollectionStorage::const_iterator storageIter = profiler.storage.begin();
		for (; storageIter != profiler.storage.end(); ++storageIter)
		{
			const SizeType threadId = storageIter->first;
// 			if (threadId != 0) continue;

			threadText.setString(TS_FMT(
				"Thread #%u (%s)",
				threadId, profiler.threadNames[threadId]
			));

			const math::VC2 basePosition(view.size.x, 50.f + threadId * 220.f + height + displayOffset.y);

			threadText.setPosition(
				50.f,
				basePosition.y - height);

			renderTarget.draw(threadText);

			const ZoneFrameCollection &collection = storageIter->second;

			if (threadId == 0)
			{
				ZoneFrameCollection::const_reverse_iterator frameIter = collection.rbegin();
				for (; frameIter != collection.rend(); ++frameIter)
				{
					const ZoneFrame &frame = *frameIter;
					float frameBaseWidth = frame.front().elapsed / timescale;

					if (offsetAccumulator + frameBaseWidth >= displayOffset.x)
					{
						startTimeLimit = frame.front().start;

						timeDisplayOffset = displayOffset.x - offsetAccumulator;

						break;
					}

					offsetAccumulator += frameBaseWidth;
				}
			}

			ZoneFrameCollection::const_reverse_iterator frameIter = collection.rbegin();
			for (; frameIter != collection.rend(); ++frameIter)
			{
				const ZoneFrame &frame = *frameIter;
				PosType frameIndex = std::distance(collection.begin(), frameIter.base());
				PosType frameIndexInverted = std::distance(collection.rbegin(), frameIter);

				int64 startTime = math::max<int64>(0, frame.front().start);

				int64 absoluteTimeDiff = baseStartTime - startTime;
				float absoluteOffsetX = -(absoluteTimeDiff / timescale) + displayOffset.x;

				if (absoluteOffsetX > 0.f)
					continue;

				float frameBaseWidth = frame.front().elapsed / timescale;
				
				if (math::abs(absoluteOffsetX) - frameBaseWidth >= view.size.x)
					break;

				math::VC2 positionOffset(absoluteOffsetX, 0.f);

				if (frameBaseWidth >= 5.f)
				{
					labels.push_back({
						math::FloatRect(basePosition + positionOffset, math::VC2(frameBaseWidth, 20.f), math::Rect_PositionAndSize),
						TS_FMT("%lld", frameIndex)
					});
				}

				for (ZoneFrame::const_iterator eventIter = frame.begin(); eventIter != frame.end(); ++eventIter)
				{
					const ZoneEvent &event = *eventIter;

					float offsetX = (event.level == 0 ? 0 : event.start) / timescale;
					float frameWidth = math::max(2.f, event.elapsed / timescale - margin);

					math::VC2 pos = basePosition + positionOffset
						+ math::VC2(offsetX, (event.level + 1.f) * (height + margin));

					math::VC2 size = math::VC2(frameWidth, height);

					math::FloatRect rect(pos, size, math::Rect_PositionAndSize);

					sf::Color color = event.level < colors.size() ? colors[event.level] : sf::Color::Magenta;
					if (rect.isPointWithin(mousePosition))
					{
						color += sf::Color(30, 30, 30);
						hoverInfo = { threadId, profiler.threadNames[threadId], frameIndex, &frame, &event };
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
							event.name
						});
					}
				}
			}

		}

		renderTarget.draw(bars);

		for (const Label &lbl : labels)
		{
			threadText.setString(lbl.text);
			threadText.setPosition(lbl.rect.minbounds);

			sf::Shader &shader = *clipShader->getResource();
			shader.setUniform("u_resolution", view.size);
			shader.setUniform("u_position", lbl.rect.minbounds);
			shader.setUniform("u_size", lbl.rect.getSize());

			renderTarget.draw(threadText, &shader);
		}

		if (hoverInfo.isValid())
		{
			int64 startTime = hoverInfo.event->start + (hoverInfo.event->level > 0 ? hoverInfo.frame->front().start : 0);
			TimeSpan start = TimeSpan::fromMicroseconds(startTime);
			TimeSpan elapsed = TimeSpan::fromMicroseconds(hoverInfo.event->elapsed);

			threadText.setString(TS_FMT(
				"Thread #%u (%s) - Frame %lld\n"
				"%s\n"
				"Start     : %s\n"
				"Elapsed : %s"
				,
				hoverInfo.threadId, hoverInfo.threadName, hoverInfo.frameIndex,
				hoverInfo.event->name,
				start.getAsString(),
				elapsed.getAsString()
			));

			static const math::VC2 offset = math::VC2(15.f, 0.f);
			static const math::VC2 padding = math::VC2(10.f, 10.f);

			threadText.setPosition(mousePosition + padding / 2.f + offset);

			math::FloatRect bounds = threadText.getLocalBounds();
			sf::RectangleShape bg(bounds.getSize() + padding);
			bg.setPosition(mousePosition + offset);

			bg.setFillColor(sf::Color(0, 0, 0, 150));
			bg.setOutlineThickness(1.f);
			bg.setOutlineColor(sf::Color(255, 255, 255, 100));

			renderTarget.draw(bg);
			renderTarget.draw(threadText);
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
			sf::Color(185, 75, 255),
		})
	{
		system::BaseApplication &app = TS_GET_GIGATON().getGigaton<system::BaseApplication>();
		debugFont = &app.getDebugFont();

		windowManager = &TS_GET_GIGATON().getGigaton<system::WindowManager>();

		clipShader = makeShared<resource::ShaderResource>("shader/area_clip.frag");
		if (clipShader == nullptr || !clipShader->loadResource())
		{
			TS_ASSERT(!"Failed to load text clip shader.");
		}
	}

	const std::vector<sf::Color> colors;

	bool visible = false;
	sf::Font *debugFont = nullptr;
	SharedPointer<resource::ShaderResource> clipShader;
	math::VC2 mousePosition;

	system::WindowManager *windowManager = nullptr;
};

ZoneProfilerRenderer &ZoneProfilerRenderer::get()
{
	static ZoneProfilerRenderer instance;
	return instance;
}

std::recursive_mutex ZoneProfiler::mutex;
std::atomic_bool ZoneProfiler::saving = false;

std::atomic_bool ZoneProfiler::enabled = false;

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
#if TS_PROFILER_ENABLED == TS_TRUE
	ZoneProfiler &instance = get();

	std::unique_lock<std::recursive_mutex> lock(mutex);
	if (instance.storage.empty())
	{
		TS_LOG_WARNING("Zone Profiler is has no profiling data to save.\n");
		saving = false;
		return;
	}

	std::map<SizeType, ZoneFrameCollection> storage = instance.storage;

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

		ZoneFrameCollection &collection = storageIter->second;
		
		numFrames += collection.size();

		for (SizeType frameIndex = 0; frameIndex < collection.size(); ++frameIndex)
		{
			const ZoneFrame &frame = collection[frameIndex];

			WRITE("<frame start=\"%lld\" elapsed=\"%lld\">\n", frame.front().start, frame.front().elapsed);
			indent++;

			numEvents += frame.size();

			int32 openings = 0;
			for (auto eventsIter = frame.begin(); eventsIter != frame.end(); ++eventsIter)
			{
				const ZoneEvent &event = *eventsIter;

				int32 level = event.level;

				auto nextIter = std::next(eventsIter);
				int32 nextLevel = nextIter == frame.end() ? 0 : nextIter->level;

				bool closedTag = (nextLevel <= level);

				if (event.isMutexEvent)
				{
					if (event.wasBlocked)
					{
						WRITE("<mutex start=\"%lld\" elapsed=\"%lld\" name=\"%s\" blockedBy=\"%u\" />\n",
							level == 0 ? 0 : event.start,
							event.elapsed,
							event.name,
							event.mutexOwner
						);
					}
					else
					{
						WRITE("<mutex start=\"%lld\" elapsed=\"%lld\" name=\"%s\" />\n",
							level == 0 ? 0 : event.start,
							event.elapsed,
							event.name
						);
					}
				}
				else
				{
					WRITE("<event start=\"%lld\" elapsed=\"%lld\" name=\"%s\"%s>\n",
						level == 0 ? 0 : event.start,
						event.elapsed,
						event.name,
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
#endif
}

#undef WRITE

void ZoneProfiler::commit(ZoneFrame &&frame)
{
	static thread_local SizeType threadId = thread::CurrentThread::getThreadId();
	static thread_local std::string threadName = thread::CurrentThread::getThreadName();
	static thread_local bool threadNameSet = false;

	if (ZoneProfiler::enabled)
	{
		std::unique_lock<std::recursive_mutex> lock(mutex);

		ZoneProfiler &instance = get();

		TS_ASSERT(!frame.empty());
		instance.storage[threadId].push_back(std::move(frame));

		if (!threadNameSet)
		{
			instance.threadNames[threadId] = threadName;
			threadNameSet = true;
		}

// 		if (instance.storage[0].size() >= 100)
// 			ZoneProfiler::enabled = false;
	}
	else
	{
		frame.clear();
	}
}

void ZoneProfiler::setEnabled(const bool enabledParam)
{
#if TS_PROFILER_ENABLED == TS_TRUE
	enabled = enabledParam;
#endif
}

bool ZoneProfiler::isEnabled()
{
#if TS_PROFILER_ENABLED == TS_TRUE
	return enabled.load();
#else
	return false;
#endif
}

void ZoneProfiler::toggleVisibility()
{
#if TS_PROFILER_ENABLED == TS_TRUE
	ZoneProfilerRenderer::get().toggleVisibility();
#endif
}

bool ZoneProfiler::isVisible()
{
#if TS_PROFILER_ENABLED == TS_TRUE
	return ZoneProfilerRenderer::get().isVisible();
#else
	return false;
#endif
}

bool ZoneProfiler::handleEvent(const sf::Event &event)
{
#if TS_PROFILER_ENABLED == TS_TRUE
	return ZoneProfilerRenderer::get().handleEvent(event);
#else
	return false;
#endif
}

void ZoneProfiler::render(sf::RenderTarget &renderTarget, const system::WindowView &view)
{
#if TS_PROFILER_ENABLED == TS_TRUE
	ZoneProfilerRenderer::get().render(renderTarget, view);
#endif
}

int64 ScopedZoneTimer::absoluteStartTime = -1;

thread_local int64 ScopedZoneTimer::frameStartTime = 0;
thread_local ZoneFrame ScopedZoneTimer::currentFrame;
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

	ZoneEvent event = {
		name,
		eventLevel == 0 ? frameStartTime - absoluteStartTime : startTime - frameStartTime,
		-1,
		(int32)eventLevel,
		0, false, false
	};

	if (currentFrame.capacity() == 0)
		currentFrame.reserve(8);

	currentFrame.push_back(std::move(event));
	frameIndex = currentFrame.size() - 1;

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

	ZoneEvent event = {
		name,
		eventLevel == 0 ? frameStartTime - absoluteStartTime : startTime - frameStartTime,
		-1,
		(int32)eventLevel,
		mutexOwner, true, blocked
	};

	currentFrame.push_back(std::move(event));
	frameIndex = currentFrame.size() - 1;

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

	ZoneEvent &event = currentFrame[frameIndex];

	event.elapsed = elapsed.getMicroseconds();
	TS_ASSERT(event.elapsed >= 0);

	if (eventLevel == 0)
	{
		ZoneProfiler::commit(std::move(currentFrame));
		TS_ASSERT(currentFrame.empty());

	}
// 	else
// 	{
// 		EventList currentFrameCopy = currentFrame;
// 		ZoneProfiler::commit(std::move(currentFrameCopy));
// 
// 		currentFrame.erase(std::next(currentFrame.begin(), eventLevel), currentFrame.end());
// 	}
	
	committed = true;
}

TS_END_PACKAGE1()


