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

	bool addArchive(const std::string &archivePath);

	bool fileExists(const std::string &filepath);
	PosType getFileSize(const std::string &filepath);

	const std::vector<std::string> &getFileList() const;
	bool getFileExtractor(const std::string &filepath, ArchivistReaderExtractor &extractor);

// 	void setFallbackPath(const std::string &fallbackPath);
// 	void setOverridePath(const std::string &overridePath);

private:
// 	std::string fallbackPath;
// 	std::string overridePath;

	std::map<SizeType, UniquePointer<ArchivistReader>> archives;

	// Maps file hash to archive hash
	std::map<SizeType, SizeType> fileToArchiveMapping;

	std::vector<std::string> filelist;
};

TS_END_PACKAGE1()

