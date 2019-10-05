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
	m_impl.reset(new FileWatcherWindows(this));
	return m_impl->watch(path, watchRecursive, flags);
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
