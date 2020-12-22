#pragma once

TS_PACKAGE2(resource, archivist)

enum ArchivistCompressionMode : uint16_t
{
	CompressionType_NoCompression  = 0,
	CompressionType_LZ4FullBlock   = 1,
	CompressionType_LZ4Streaming   = 2,
};

struct ArchivistFileHeader
{
	char filename[96];
	SizeType filesize;
	SizeType offset;
	uint16_t compression;
};

struct ArchivistFileFormat
{
	char formatString[4];
	SizeType version;
};
#define TS_ARCHIVIST_FORMAT_STRING "TSP"

struct ArchivistConstants
{
	enum
	{
		DictionarySize = 1024 * 64,

		CompressionBlockSize = 1024 * 128,
		OverheadPerBlock = 8,
	};
};

TS_END_PACKAGE2()
