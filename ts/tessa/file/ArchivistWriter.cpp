#include "Precompiled.h"
#include "ts/tessa/file/ArchivistWriter.h"

#include "lz4.h"

#include "ts/tessa/file/InputFile.h"
#include "ts/tessa/file/OutputFile.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/tessa/math/CRC.h"
#include "ts/tessa/math/Hash.h"

TS_PACKAGE1(file)

namespace
{

const SizeType ArchivistWriterFormatVersion = 1;

}

ArchivistWriter::ArchivistWriter()
{

}

bool ArchivistWriter::stageFile(const std::string &filepath, const std::string &archiveFilepath, ArchivistCompressionMode compression)
{
	const SizeType hash = math::simpleHash32(filepath);
	if (stagefiles.count(hash) > 0)
	{
		TS_LOG_WARNING("File is already staged. File: %s\n", filepath);
		return false;
	}

	if (!utils::exists(filepath))
	{
		TS_LOG_ERROR("File to be staged does not exist. File: %s\n", filepath);
		return false;
	}

	if (!utils::isFile(filepath))
	{
		TS_LOG_ERROR("File path to be staged is not a file (directories are not supported). File: %s\n", filepath);
		return false;
	}

	Stagefile &file = stagefiles[hash];
	file.filepath = std::move(filepath);
	file.archiveFilepath = utils::normalizePath(archiveFilepath, '/');
	file.compression = compression;
	return true;
}

bool ArchivistWriter::saveToFile(const std::string &archiveFilename, bool overwriteExisting)
{
	if (utils::exists(archiveFilename) && overwriteExisting == false)
	{
		TS_LOG_ERROR("Archive file already exists with the same name. File: %s", archiveFilename);
		return false;
	}

	std::vector<ArchivistFileHeader> headers;
	for (StagefileList::const_iterator it = stagefiles.begin(); it != stagefiles.end(); ++it)
	{
		const Stagefile &file = it->second;

		InputFile input;
		if (!input.open(file.filepath, InputFileMode_ReadBinary))
		{
			TS_LOG_ERROR("Unable to open file for reading. File: %s\n", file.filepath);
			return false;
		}

		BigSizeType filesize = input.getFileSize();
		if (filesize > MaxFileSize)
		{
			TS_LOG_ERROR("File is too large, maximum size is %u MB. File: %s\n", (MaxFileSize / 1024 / 1024), file.filepath);
			return false;
		}

		input.close();

		ArchivistFileHeader header;
		memset(&header, 0, sizeof(ArchivistFileHeader));
		
		memcpy(header.filename, (void*)file.archiveFilepath.c_str(), file.archiveFilepath.size());
		header.filesize = (SizeType)filesize;
		header.compression = file.compression;

		headers.push_back(header);
	}

	OutputFile archive(archiveFilename, OutputFileMode_WriteBinaryTruncate);
	if (!archive)
		return false;

	ArchivistFileFormat archiveFormat;
	memcpy(archiveFormat.formatString, TS_ARCHIVIST_FORMAT_STRING, sizeof(TS_ARCHIVIST_FORMAT_STRING));
	archiveFormat.version = ArchivistWriterFormatVersion;
	archive.writeVariable(archiveFormat);

	PosType headerBeginPos = archive.tell();

	archive.writeVariable((SizeType)headers.size());

	SizeType headerBlockSize = (SizeType)(headers.size() * sizeof(ArchivistFileHeader));
	archive.write(reinterpret_cast<const char*>(&headers[0]), headerBlockSize);

	PosType fileBlockStart = archive.tell();
	PosType currentOffset = fileBlockStart;

	ByteBuffer buffer;

	SizeType index = 0;
	for (StagefileList::const_iterator it = stagefiles.begin(); it != stagefiles.end(); ++it, ++index)
	{
		const Stagefile &file = it->second;

		PosType bufferBytesWritten = 0;
		switch (file.compression)
		{
			case CompressionType_NoCompression:
				bufferBytesWritten = copyFileToBuffer(file.filepath, buffer);
			break;
			case CompressionType_LZ4Compression:
				bufferBytesWritten = lz4_compressFileToBuffer(file.filepath, buffer);
			break;
		}

		if (bufferBytesWritten < 0)
		{
			TS_LOG_ERROR("Error copying/compressing to buffer.\n");
			return false;
		}

		headers[index].offset = (SizeType)currentOffset;

		archive.write(&buffer[0], bufferBytesWritten);
		currentOffset += bufferBytesWritten;
	}

	// Rewrite headers with updated offsets
	archive.seek(headerBeginPos + sizeof(SizeType));
	archive.write(reinterpret_cast<const char*>(&headers[0]), headerBlockSize);

	archive.close();

	return false;
}

PosType ArchivistWriter::copyFileToBuffer(const std::string &filepath, ByteBuffer &dstBuffer)
{
	InputFile input;
	if (!input.open(filepath, InputFileMode_ReadBinary))
		return false;

	PosType filesize = input.getFileSize();
	dstBuffer.resize(filesize);

	PosType bytesRead = input.read(&dstBuffer[0], filesize);
	if (bytesRead < filesize)
	{
		dstBuffer.clear();
		return -1;
	}
	return bytesRead;
}

PosType ArchivistWriter::lz4_compressFileToBuffer(const std::string &filepath, ByteBuffer &dstBuffer)
{
	InputFile input;
	if (!input.open(filepath, InputFileMode_ReadBinary))
		return false;

	PosType filesize = input.getFileSize();

	const SizeType blockSizeMax = LZ4_compressBound(ArchivistConstants::CompressionBlockSize);

	SizeType numBlocks = (SizeType)math::ceil(filesize / (float)blockSizeMax) +1;
	Int32 dstBoundSize = numBlocks * (blockSizeMax + ArchivistConstants::OverheadPerBlock) + sizeof(SizeType);
	dstBuffer.resize(dstBoundSize);

	char *TS_RESTRICT dstStart = &dstBuffer[0];
	char *TS_RESTRICT dstPtr = dstStart;
	memset(dstStart, 0, dstBoundSize);

	LZ4_stream_t lz4Stream_body;
	LZ4_stream_t *lz4Stream = &lz4Stream_body;
	LZ4_initStream(lz4Stream, sizeof(*lz4Stream));

	SizeType srcBufferIndex = 0;
	char srcDoubleBuffer[2][ArchivistConstants::CompressionBlockSize] = { 0 };

// 	TS_PRINTF("File %s (total size %lld bytes):\n", filepath, filesize);

	SizeType blocks = 0;
	while (true)
	{
// 		TS_PRINTF("Block %u:\n", blocks);

		char *srcPtr = srcDoubleBuffer[srcBufferIndex];
		PosType srcBytesRead = input.read(srcPtr, ArchivistConstants::CompressionBlockSize);
		if (srcBytesRead <= 0)
			break;

// 		TS_PRINTF("  block size offset : %lld\n", dstPtr - dstStart);

		SizeType *blockCompressedSize = reinterpret_cast<SizeType*>(dstPtr);
		dstPtr += sizeof(SizeType);

// 		TS_PRINTF("  crc offset        : %lld\n", dstPtr - dstStart);

		SizeType *blockCRC = reinterpret_cast<SizeType*>(dstPtr);
		dstPtr += sizeof(SizeType);

		PosType dstBytesRemaining = dstBoundSize - (dstPtr - dstStart);
		TS_ASSERT(dstBytesRemaining > 0 && "dst buffer is out of space");

// 		TS_PRINTF("  data offset       : %lld\n", dstPtr - dstStart);

		const Int32 compressedBytes = LZ4_compress_fast_continue(lz4Stream, srcPtr, dstPtr, (Int32)srcBytesRead, (Int32)dstBytesRemaining, 1);
		if (compressedBytes <= 0)
		{
			TS_LOG_ERROR("Compression encountered an error.\n");
			dstBuffer.clear();
			return -1;
		}

		*blockCompressedSize = compressedBytes;
		*blockCRC = math::crc32(srcPtr, srcBytesRead);

// 		TS_PRINTF("  compressed %lld => %d (CRC32 %08X)\n", srcBytesRead, compressedBytes, *blockCRC);

		blocks++;

		srcBufferIndex = (srcBufferIndex + 1) % 2;
		dstPtr += compressedBytes;

// 		TS_PRINTF("  block end offset  : %lld\n", dstPtr - dstStart);
	}

	// Finalize the file block with a zero which will be read as zero size block by the decompressor
	*reinterpret_cast<SizeType*>(dstPtr) = 0;
	dstPtr += sizeof(SizeType);

	PosType dstBytesWritten = (dstPtr - dstStart);
	dstBuffer.resize(dstBytesWritten);

	return dstBytesWritten;
}

TS_END_PACKAGE1()
