#include "Precompiled.h"
#include "ts/tessa/file/ArchivistFilesystem.h"

#include <algorithm>

#include "ts/tessa/file/ArchivistReader.h"
#include "ts/tessa/file/ArchivistReaderExtractor.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/tessa/math/Hash.h"

TS_DEFINE_MANAGER_TYPE(file::ArchivistFilesystem);

TS_PACKAGE1(file)

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

bool ArchivistFilesystem::addArchive(const std::string &archivePath)
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
	std::vector<std::string> list = reader->getFileList();
	for (std::string &filepath : list)
	{
		const SizeType fileHash = math::simpleHash32(filepath);

		if (fileToArchiveMapping.count(fileHash) > 0)
			TS_PRINTF("%s is being overridden by another file from %s\n", filepath, archivePath);

		fileToArchiveMapping[fileHash] = archiveHash;

		if (std::find(filelist.begin(), filelist.end(), filepath) == filelist.end())
			filelist.push_back(filepath);
	}

	archives[archiveHash] = std::move(reader);

	return true;
}

bool ArchivistFilesystem::fileExists(const std::string &filepath)
{
// 	if (!overridePath.empty() && utils::exists(utils::joinPaths(overridePath, filepath)))
// 		return true;

	const SizeType fileHash = math::simpleHash32(filepath);
	if (fileToArchiveMapping.count(fileHash) > 0)
		return true;

// 	if (!fallbackPath.empty() && utils::exists(utils::joinPaths(fallbackPath, filepath)))
// 		return true;

	return false;
}

PosType ArchivistFilesystem::getFileSize(const std::string &filepath)
{
	const SizeType fileHash = math::simpleHash32(filepath);
	if (fileToArchiveMapping.count(fileHash) > 0)
	{
		const SizeType archiveHash = fileToArchiveMapping[fileHash];
		return archives[archiveHash]->getFileSize(filepath);
	}
	return 0;
}

const std::vector<std::string> &ArchivistFilesystem::getFileList() const 
{
	return filelist;
}

bool ArchivistFilesystem::getFileExtractor(const std::string &filepath, ArchivistReaderExtractor &extractor)
{
	const SizeType fileHash = math::simpleHash32(filepath);
	if (fileToArchiveMapping.count(fileHash) > 0)
	{
		const SizeType archiveHash = fileToArchiveMapping[fileHash];
		return archives[archiveHash]->getFileExtractor(filepath, extractor);
	}
	return false;
}

// void ArchivistFilesystem::setFallbackPath(const std::string &fallbackPathParam)
// {
// 	fallbackPath = fallbackPathParam;
// }
// 
// void ArchivistFilesystem::setOverridePath(const std::string &overridePathParam)
// {
// 	overridePath = overridePathParam;
// }


TS_END_PACKAGE1()
