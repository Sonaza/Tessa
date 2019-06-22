#pragma once

#include <string>
#include <vector>

#include "ts/tessa/file/ArchivistCommon.h"
#include "ts/tessa/file/InputFile.h"

TS_DECLARE1(file, ArchivistReader);

TS_PACKAGE1(file)

class ArchivistReader;

class ArchivistReaderExtractor
{
public:
	ArchivistReaderExtractor();
	~ArchivistReaderExtractor();

	void close();

	PosType read(char *outBuffer, BigSizeType size);

	PosType seek(PosType pos);
	PosType tell() const;

	SizeType getSize() const;

	bool isEOF() const;

private:
	friend class ArchivistReader;

	PosType readNoCompressed(char *outBuffer, BigSizeType size);
	PosType readLZ4Compressed(char *outBuffer, BigSizeType size);

	PosType decompressBlocks(SizeType numBlocks);

	bool initialize(const ArchivistFileHeader &header, const std::string &archiveFilepath);

	bool initialized = false;
	bool eof = false;
	bool decompressionComplete = false;

	InputFile archiveFile;
	ArchivistFileHeader header;

	std::vector<PosType> blockOffsets;
	SizeType numDecompressedBlocks = 0;

	void *streamDecode = nullptr;
	std::vector<char> decompressedBuffer;

	PosType currentPosition = 0;

};

TS_END_PACKAGE1()

