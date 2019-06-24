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

	bool stageFile(const std::string &filepath, const std::string &archiveFilepath, ArchivistCompressionMode compression);
	bool saveToFile(const std::string &filename, bool overwriteExisting = true);

private:
	typedef std::vector<char> ByteBuffer;

	PosType copyFileToBuffer(const std::string &filepath, ByteBuffer &dstBuffer);
	PosType lz4_compressFullBlockFileToBuffer(const std::string &filepath, ByteBuffer &dstBuffer);
	PosType lz4_compressStreamedFileToBuffer(const std::string &filepath, ByteBuffer &dstBuffer);

	struct Stagefile
	{
		std::string filepath;
		std::string archiveFilepath;
		SizeType filesize;
		ArchivistCompressionMode compression;
	};
	typedef std::map<SizeType, Stagefile> StagefileList;
	StagefileList _stagefiles;

	BigSizeType _stagedTotalFilesize = 0;

	const BigSizeType FileMaxSize = std::numeric_limits<int32_t>::max();
	const BigSizeType ArchiveMaxSize = std::numeric_limits<int32_t>::max();
};

TS_END_PACKAGE1()

