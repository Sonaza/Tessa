#include "Precompiled.h"
#include "ArchivistInputStream.h"

TS_PACKAGE2(resource, archivist)

ArchivistInputStream::ArchivistInputStream(ArchivistReaderExtractor &extractorParam)
	: extractor(std::move(extractorParam))
{
	TS_ASSERT(extractor.isGood());
}

ArchivistInputStream::~ArchivistInputStream()
{
	close();
}

void ArchivistInputStream::close()
{
	extractor.close();
}

PosType ArchivistInputStream::read(void* data, PosType size)
{
	TS_ASSERT(extractor.isGood());
	return extractor.read((char*)data, size);
}

PosType ArchivistInputStream::seek(PosType position)
{
	TS_ASSERT(extractor.isGood());
	return extractor.seek(position);
}

PosType ArchivistInputStream::tell()
{
	TS_ASSERT(extractor.isGood());
	return extractor.tell();
}

PosType ArchivistInputStream::getSize()
{
	TS_ASSERT(extractor.isGood());
	return extractor.getSize();
}

TS_END_PACKAGE2()
