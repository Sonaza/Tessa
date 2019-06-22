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

	bool openArchive(const std::string &filepath);

	bool fileExists(const std::string &filename) const;
	PosType getFileSize(const std::string &filename);
	std::vector<std::string> getFileList() const;

	SizeType getNumFiles() const;

	bool extractToFile(const std::string &filename, const std::string &targetFilepath);

	bool getFileExtractor(const std::string &filename, ArchivistReaderExtractor &extractor);

private:
// 	mutable std::mutex mutex;

	typedef std::vector<char> ByteBuffer;

	file::InputFile archive;

	typedef std::map<SizeType, ArchivistFileHeader> ArchivistFileHeaders;
	ArchivistFileHeaders headers;
	std::string archiveFilepath;
};

TS_END_PACKAGE1()

