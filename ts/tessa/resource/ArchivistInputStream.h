#pragma once

#include "ts/tessa/file/ArchivistReaderExtractor.h"

TS_PACKAGE1(resource)

class ArchivistInputStream : public sf::InputStream
{
public:
	ArchivistInputStream(file::ArchivistReaderExtractor &extractor);
	~ArchivistInputStream();

	void close();

	virtual PosType read(void* data, PosType size);
	virtual PosType seek(Int64 position);
	virtual PosType tell();
	virtual PosType getSize();

private:
	file::ArchivistReaderExtractor extractor;

};

TS_END_PACKAGE1()
