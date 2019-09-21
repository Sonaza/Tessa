#pragma once

#include <string>
#include <vector>
#include <map>
#include <limits>

#include "ts/tessa/file/ArchivistCommon.h"

TS_PACKAGE1(file)

class ArchivistWriter : public lang::Noncopyable
{
public:
	ArchivistWriter();

	bool stageFile(const String &filepath, const String &archiveFilepath, ArchivistCompressionMode compression);
	bool saveToFile(const String &filename, bool overwriteExisting = true);

private:
	typedef std::vector<char> ByteBuffer;

	PosType copyFileToBuffer(const String &filepath, ByteBuffer &dstBuffer);
	PosType lz4_compressFullBlockFileToBuffer(const String &filepath, ByteBuffer &dstBuffer);
	PosType lz4_compressStreamedFileToBuffer(const String &filepath, ByteBuffer &dstBuffer);

	struct Stagefile
	{
		String filepath;
		String archiveFilepath;
		SizeType filesize;
		ArchivistCompressionMode compression;
	};
	typedef std::map<SizeType, Stagefile> StagefileList;
	StagefileList stagefiles;

	BigSizeType totalStagedFilesize = 0;

	const BigSizeType FileMaxSize = std::numeric_limits<int32_t>::max() - 1024;
	const BigSizeType ArchiveMaxSize = std::numeric_limits<int32_t>::max();
};

TS_END_PACKAGE1()

