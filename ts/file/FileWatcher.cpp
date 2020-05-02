#include "Precompiled.h"
#include "FileWatcher.h"

#if TS_PLATFORM == TS_WINDOWS
	#include "ts/file/windows/FileWatcherWindows.h"
#endif

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
#if TS_PLATFORM == TS_WINDOWS
	
	m_impl.reset(new FileWatcherWindows(this));
	return m_impl->watch(path, watchRecursive, flags);

#elif TS_PLATFORM == TS_LINUX
	
	// TODO
	return false;
	
#else

	TS_ASSERT(false && "Not implemented on this platform");
	return false;

#endif
}

void FileWatcher::reset()
{
	if (m_impl)
		m_impl->reset();
}

void FileWatcher::update()
{
	if (m_impl)
		m_impl->update();
}

bool FileWatcher::isWatching() const
{
	return m_impl != nullptr && m_impl->isWatching();
}

bool FileWatcher::hasError() const
{
	return m_impl != nullptr && m_impl->hasError();
}

TS_END_PACKAGE1()
