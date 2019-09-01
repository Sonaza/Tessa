#pragma once

#include "ts/tessa/system/AbstractManagerBase.h"

#include <string>
#include <vector>
#include <map>

TS_DECLARE1(file, ArchivistReader);
TS_DECLARE1(file, ArchivistReaderExtractor);

TS_PACKAGE1(file)

class ArchivistFilesystem : public system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(file::ArchivistFilesystem);

public:
	ArchivistFilesystem();
	virtual ~ArchivistFilesystem();

	virtual bool initialize();
	virtual void deinitialize();

	virtual void update(const TimeSpan deltaTime);

	bool addArchive(const String &archivePath);

	bool fileExists(const String &filepath);
	PosType getFileSize(const String &filepath);

	const std::vector<String> &getFileList() const;
	bool getFileExtractor(const String &filepath, ArchivistReaderExtractor &extractor);

// 	void setFallbackPath(const String &fallbackPath);
// 	void setOverridePath(const String &overridePath);

private:
// 	String fallbackPath;
// 	String overridePath;

	std::map<SizeType, UniquePointer<ArchivistReader>> archives;

	// Maps file hash to archive hash
	std::map<SizeType, SizeType> fileToArchiveMapping;

	std::vector<String> filelist;
};

TS_END_PACKAGE1()

