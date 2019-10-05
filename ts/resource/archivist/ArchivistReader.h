#pragma once

#include <string>
#include <vector>
#include <map>

#include "ts/resource/archivist/ArchivistCommon.h"

#include "ts/file/InputFile.h"
#include "ts/file/OutputFile.h"

TS_DECLARE2(resource, archivist, ArchivistReaderExtractor);

TS_PACKAGE2(resource, archivist)

class ArchivistReader : public lang::Noncopyable
{
public:
	ArchivistReader();

	bool openArchive(const String &filepath);

	bool fileExists(const String &filename) const;
	PosType getFileSize(const String &filename);

	std::vector<String> getFileList() const;
	SizeType getNumFiles() const;

	bool getFileExtractor(const String &filename, ArchivistReaderExtractor &extractor);
	bool extractToFile(const String &filename, const String &targetFilepath);

private:
	typedef std::map<SizeType, ArchivistFileHeader> ArchivistFileHeaders;
	ArchivistFileHeaders headers;
	String archiveFilepath;
};

TS_END_PACKAGE2()

