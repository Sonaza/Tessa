#pragma once

#include "ts/resource/archivist/ArchivistReaderExtractor.h"

#include "SFML/System/InputStream.hpp"

TS_PACKAGE2(resource, archivist)

class ArchivistInputStream : public sf::InputStream
{
public:
	ArchivistInputStream(ArchivistReaderExtractor &extractor);
	~ArchivistInputStream();

	void close();

	virtual PosType read(void* data, sf::Int64 size);
	virtual PosType seek(sf::Int64 position);
	virtual PosType tell();
	virtual PosType getSize();

private:
	ArchivistReaderExtractor extractor;

};

TS_END_PACKAGE2()
