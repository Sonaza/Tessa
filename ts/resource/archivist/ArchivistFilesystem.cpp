#include "Precompiled.h"
#include "ArchivistFilesystem.h"

#include "ts/resource/archivist/ArchivistReader.h"
#include "ts/resource/archivist/ArchivistReaderExtractor.h"
#include "ts/file/FileUtils.h"
#include "ts/math/Hash.h"
#include <algorithm>

TS_DEFINE_MANAGER_TYPE(resource::archivist::ArchivistFilesystem);

TS_PACKAGE2(resource, archivist)

ArchivistFilesystem::ArchivistFilesystem()
{
	gigaton.registerClass(this);
}

ArchivistFilesystem::~ArchivistFilesystem()
{
	gigaton.unregisterClass(this);
}

bool ArchivistFilesystem::initialize()
{
	return true;
}

void ArchivistFilesystem::deinitialize()
{

}

void ArchivistFilesystem::update(const TimeSpan deltaTime)
{

}

bool ArchivistFilesystem::addArchive(const String &archivePath)
{
	const SizeType archiveHash = math::simpleHash32(archivePath);
	if (archives.count(archiveHash) > 0)
	{
		TS_LOG_WARNING("Archive is already added to the filesystem. File: %s\n", archivePath);
		return false;
	}

	UniquePointer<ArchivistReader> reader = makeUnique<ArchivistReader>();
	TS_ASSERT(reader);

	if (!reader->openArchive(archivePath))
	{
		TS_LOG_ERROR("Failed to open archive. File: %s\n", archivePath);
		return false;
	}

	// Updates and overrides the file mapping so that any time
	// a file is requested it is loaded from the latest archive.
	std::vector<String> list = reader->getFileList();
	for (String &filepath : list)
	{
		const SizeType fileHash = math::simpleHash32(filepath.toUtf8());

		if (fileToArchiveMapping.count(fileHash) > 0)
			TS_PRINTF("%s is being overridden by another file from %s\n", filepath, archivePath);

		fileToArchiveMapping[fileHash] = archiveHash;

		if (std::find(filelist.begin(), filelist.end(), filepath) == filelist.end())
			filelist.push_back(filepath);
	}

	archives[archiveHash] = std::move(reader);

	return true;
}

bool ArchivistFilesystem::fileExists(const String &filepath)
{
// 	if (!overridePath.empty() && exists(joinPaths(overridePath, filepath)))
// 		return true;

	const SizeType fileHash = math::simpleHash32(filepath.toUtf8());
	if (fileToArchiveMapping.count(fileHash) > 0)
		return true;

// 	if (!fallbackPath.empty() && exists(joinPaths(fallbackPath, filepath)))
// 		return true;

	return false;
}

PosType ArchivistFilesystem::getFileSize(const String &filepath)
{
	const SizeType fileHash = math::simpleHash32(filepath.toUtf8());
	if (fileToArchiveMapping.count(fileHash) > 0)
	{
		const SizeType archiveHash = fileToArchiveMapping[fileHash];
		return archives[archiveHash]->getFileSize(filepath);
	}
	return 0;
}

const std::vector<String> &ArchivistFilesystem::getFileList() const 
{
	return filelist;
}

bool ArchivistFilesystem::getFileExtractor(const String &filepath, ArchivistReaderExtractor &extractor)
{
	const SizeType fileHash = math::simpleHash32(filepath.toUtf8());
	if (fileToArchiveMapping.count(fileHash) > 0)
	{
		const SizeType archiveHash = fileToArchiveMapping[fileHash];
		return archives[archiveHash]->getFileExtractor(filepath, extractor);
	}
	return false;
}

// void ArchivistFilesystem::setFallbackPath(const String &fallbackPathParam)
// {
// 	fallbackPath = fallbackPathParam;
// }
// 
// void ArchivistFilesystem::setOverridePath(const String &overridePathParam)
// {
// 	overridePath = overridePathParam;
// }


TS_END_PACKAGE2()
