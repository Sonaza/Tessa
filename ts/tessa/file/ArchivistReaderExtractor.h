#pragma once

#include <string>
#include <vector>

#include "ts/tessa/file/ArchivistCommon.h"
#include "ts/tessa/file/InputFile.h"

TS_PACKAGE1(file)

class ArchivistReader;

class ArchivistReaderExtractor : public lang::Noncopyable
{
	friend class ArchivistReader;

public:
	ArchivistReaderExtractor();
	~ArchivistReaderExtractor();

	ArchivistReaderExtractor(ArchivistReaderExtractor &&other);
	ArchivistReaderExtractor &operator=(ArchivistReaderExtractor &&other);

	void close();

	PosType read(char *outBuffer, BigSizeType size);

	PosType seek(PosType pos);
	PosType tell() const;

	SizeType getSize() const;

	bool isGood() const;
	bool isEOF() const;

	const String getFilename() const;

private:
	typedef std::vector<char> ByteBuffer;

	bool initialize(const ArchivistFileHeader &header, const String &archiveFilepath);

	PosType readNoCompressed(char *outBuffer, BigSizeType size);

	PosType readLZ4FullBlockCompressed(char *outBuffer, BigSizeType size);
	PosType decompressFullBlock();

	PosType readLZ4StreamingCompressed(char *outBuffer, BigSizeType size);
	PosType decompressStreamingBlocks(SizeType numBlocks);

	bool initialized = false;
	bool eof = false;
	bool decompressionComplete = false;

	InputFile archiveFile;
	ArchivistFileHeader header;

	std::vector<PosType> blockOffsets;
	SizeType numDecompressedBlocks = 0;

	void *streamDecode = nullptr;
	ByteBuffer decompressedBuffer;

	PosType currentPosition = 0;

};

TS_END_PACKAGE1()

