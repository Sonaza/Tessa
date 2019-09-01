#pragma once

#include <string>
#include <vector>
#include <map>

#include "ts/tessa/file/ArchivistCommon.h"

#include "ts/tessa/file/InputFile.h"
#include "ts/tessa/file/OutputFile.h"

TS_DECLARE1(file, ArchivistReaderExtractor);

TS_PACKAGE1(file)

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

TS_END_PACKAGE1()

