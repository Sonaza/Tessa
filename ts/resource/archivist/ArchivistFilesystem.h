#pragma once

#include "ts/engine/system/AbstractManagerBase.h"

#include <string>
#include <vector>
#include <map>

TS_DECLARE2(resource, archivist, ArchivistReader);
TS_DECLARE2(resource, archivist, ArchivistReaderExtractor);

TS_PACKAGE2(resource, archivist)

class ArchivistFilesystem : public engine::system::AbstractManagerBase
{
	TS_DECLARE_MANAGER_TYPE(resource::archivist::ArchivistFilesystem);

public:
	ArchivistFilesystem();
	virtual ~ArchivistFilesystem();

	virtual bool initialize() override;
	virtual void deinitialize() override;

	virtual void update(const TimeSpan deltaTime) override;

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

TS_END_PACKAGE2()

