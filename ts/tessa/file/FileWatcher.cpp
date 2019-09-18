#include "Precompiled.h"
#include "FileWatcher.h"

#define TS_INCLUDE_FILEWATCH_IMPL

#if TS_PLATFORM == TS_WINDOWS
#include "ts/tessa/file/windows/FileWatcherWindows.h"
#endif

#undef TS_INCLUDE_FILEWATCH_IMPL

TS_PACKAGE1(file)


FileWatcher::FileWatcher()
{
	
}

FileWatcher::FileWatcher(const String &path, bool watchRecursive, SizeType flags)
{
	watch(path, watchRecursive, flags);
}

bool FileWatcher::watch(const String &path, bool watchRecursive, SizeType flags)
{
	impl.reset(new FileWatcherWindows(this));
	return impl->watch(path, watchRecursive, flags);
}

void FileWatcher::reset()
{
	if (impl)
		impl->reset();
}

void FileWatcher::update()
{
	if (impl)
		impl->update();
}

bool FileWatcher::isWatching() const
{
	return impl != nullptr && impl->isWatching();
}

TS_END_PACKAGE1()
