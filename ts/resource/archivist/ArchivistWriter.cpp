#include "Precompiled.h"
#include "ts/resource/archivist/ArchivistWriter.h"

#include "lz4.h"

#include "ts/file/InputFile.h"
#include "ts/file/OutputFile.h"
#include "ts/file/FileUtils.h"

#include "ts/math/CRC.h"
#include "ts/math/Hash.h"

TS_PACKAGE2(resource, archivist)

namespace
{

const SizeType ArchivistWriterFormatVersion = 3;

}

ArchivistWriter::ArchivistWriter()
{

}

bool ArchivistWriter::stageFile(const String &filepath, const String &archiveFilepath, ArchivistCompressionMode compression)
{
	const SizeType hash = math::simpleHash32(filepath);
	if (stagefiles.count(hash) > 0)
	{
		TS_LOG_WARNING("File is already staged. File: %s\n", filepath);
		return false;
	}

	if (!file::exists(filepath))
	{
		TS_LOG_ERROR("File to be staged does not exist. File: %s\n", filepath);
		return false;
	}

	if (!file::isFile(filepath))
	{
		TS_LOG_ERROR("File path to be staged is not a file (directories are not supported). File: %s\n", filepath);
		return false;
	}

	file::InputFile input;
	if (!input.open(filepath, file::InputFileMode_ReadBinary))
	{
		TS_LOG_ERROR("Unable to open file for reading. File: %s\n", filepath);
		return false;
	}

	BigSizeType filesize = input.getSize();
	input.close();

	// Limit individual file sizes
	if (filesize > FileMaxSize)
	{
		TS_LOG_ERROR("File size exceeds the maximum size of %u MB. File: %s\n", (FileMaxSize / 1024 / 1024), filepath);
		return false;
	}

	// Archive size would exceed the limit
	if (totalStagedFilesize + filesize > ArchiveMaxSize)
	{
		TS_LOG_ERROR("All staged files together exceed the maximum size of %u MB. File: %s\n", (ArchiveMaxSize / 1024 / 1024), filepath);
		return false;
	}

	Stagefile &file = stagefiles[hash];
	file.filepath = std::move(filepath);
	file.archiveFilepath = file::normalizePath(archiveFilepath, '/');
	file.filesize = (SizeType)filesize; // Casting is fine, file size is less than the limits
	file.compression = compression;

	totalStagedFilesize += filesize;

	return true;
}

bool ArchivistWriter::saveToFile(const String &archiveFilename, bool overwriteExisting)
{
	if (file::exists(archiveFilename) && overwriteExisting == false)
	{
		TS_LOG_ERROR("Archive file already exists with the same name. File: %s", archiveFilename);
		return false;
	}

	if (stagefiles.empty())
	{
		TS_ASSERT(!"There are no files staged for writing.");
		TS_LOG_ERROR("There are no files staged for writing.");
		return false;
	}

	std::vector<ArchivistFileHeader> headers;
	for (StagefileList::const_iterator it = stagefiles.begin(); it != stagefiles.end(); ++it)
	{
		const Stagefile &file = it->second;

		ArchivistFileHeader header;
		memset(&header, 0, sizeof(ArchivistFileHeader));
		
		auto utf8Filename = file.archiveFilepath.toUtf8();
		memcpy(header.filename, (void*)utf8Filename.c_str(), utf8Filename.size());
		header.filesize = (SizeType)file.filesize;
		header.compression = file.compression;

		headers.push_back(header);
	}

	file::OutputFile archive;
	if (!archive.open(archiveFilename, file::OutputFileMode_WriteBinaryTruncate))
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
			case CompressionType_LZ4FullBlock:
				bufferBytesWritten = lz4_compressFullBlockFileToBuffer(file.filepath, buffer);
				break;
			case CompressionType_LZ4Streaming:
				bufferBytesWritten = lz4_compressStreamedFileToBuffer(file.filepath, buffer);
			break;
		}

		if (bufferBytesWritten < 0)
		{
			TS_LOG_ERROR("Error copying/compressing to buffer.\n");
			return false;
		}

		headers[index].offset = (SizeType)currentOffset;

		archive.write(&buffer[0], (SizeType)bufferBytesWritten);
		currentOffset += bufferBytesWritten;
	}

	// Rewrite headers with updated offsets
	archive.seek(headerBeginPos + sizeof(SizeType));
	archive.write(reinterpret_cast<const char*>(&headers[0]), headerBlockSize);

	archive.close();

	return true;
}

PosType ArchivistWriter::copyFileToBuffer(const String &filepath, ByteBuffer &dstBuffer)
{
	file::InputFile input;
	if (!input.open(filepath, file::InputFileMode_ReadBinary))
		return false;

	PosType filesize = input.getSize();
	TS_ASSERT(filesize > 0);
	dstBuffer.resize(filesize);

	PosType bytesRead = input.read(&dstBuffer[0], (SizeType)filesize);
	if (bytesRead < filesize)
	{
		dstBuffer.clear();
		return -1;
	}
	return bytesRead;
}

PosType ArchivistWriter::lz4_compressFullBlockFileToBuffer(const String &filepath, ByteBuffer &dstBuffer)
{
	file::InputFile input;
	if (!input.open(filepath, file::InputFileMode_ReadBinary))
		return false;

	PosType filesize = input.getSize();
	TS_ASSERT(filesize > 0);
	ByteBuffer srcBuffer(filesize);

	const SizeType blockSizeMax = (SizeType)LZ4_compressBound((int32)filesize);
	dstBuffer.resize(blockSizeMax);

	PosType bytesRead = input.read(&srcBuffer[0], (SizeType)filesize);
	if (bytesRead < filesize)
	{
		TS_LOG_ERROR("File read error. File: %s\n", filepath);
		return -1;
	}

	char *TS_RESTRICT dstPtrStart = &dstBuffer[0];
	char *TS_RESTRICT dstPtr = dstPtrStart;

	SizeType *blockCompressedSize = reinterpret_cast<SizeType*>(dstPtr);
	dstPtr += sizeof(SizeType);

	SizeType *blockCRC = reinterpret_cast<SizeType*>(dstPtr);
	dstPtr += sizeof(SizeType);

	const int32 compressedBytes = LZ4_compress_fast(&srcBuffer[0], dstPtr, (SizeType)bytesRead, blockSizeMax, 1);
	if (compressedBytes < 0)
	{
		TS_LOG_ERROR("Compression encountered an error.\n");
		return -1;
	}

	*blockCompressedSize = compressedBytes;
	*blockCRC = math::crc32(&srcBuffer[0], filesize);

	dstPtr += compressedBytes;
	
	PosType dstBytesWritten = (dstPtr - dstPtrStart);
	dstBuffer.resize(dstBytesWritten);

	return dstBytesWritten;
}

PosType ArchivistWriter::lz4_compressStreamedFileToBuffer(const String &filepath, ByteBuffer &dstBuffer)
{
	file::InputFile input;
	if (!input.open(filepath, file::InputFileMode_ReadBinary))
		return false;

	PosType filesize = input.getSize();

	const SizeType blockSizeMax = LZ4_compressBound(ArchivistConstants::CompressionBlockSize);

	SizeType numBlocks = (SizeType)math::ceil(filesize / (float)blockSizeMax) +1;
	int32 dstBoundSize = numBlocks * (blockSizeMax + ArchivistConstants::OverheadPerBlock) + sizeof(SizeType);
	dstBuffer.resize(dstBoundSize);

	char *TS_RESTRICT dstPtrStart = &dstBuffer[0];
	char *TS_RESTRICT dstPtr = dstPtrStart;
	memset(dstPtrStart, 0, dstBoundSize);

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

// 		TS_PRINTF("  block size offset : %lld\n", dstPtr - dstPtrStart);

		SizeType *blockCompressedSize = reinterpret_cast<SizeType*>(dstPtr);
		dstPtr += sizeof(SizeType);

// 		TS_PRINTF("  crc offset        : %lld\n", dstPtr - dstPtrStart);

		SizeType *blockCRC = reinterpret_cast<SizeType*>(dstPtr);
		dstPtr += sizeof(SizeType);

		PosType dstBytesRemaining = dstBoundSize - (dstPtr - dstPtrStart);
		TS_ASSERT(dstBytesRemaining > 0 && "dst buffer is out of space");

// 		TS_PRINTF("  data offset       : %lld\n", dstPtr - dstPtrStart);

		const int32 compressedBytes = LZ4_compress_fast_continue(lz4Stream, srcPtr, dstPtr, (int32)srcBytesRead, (int32)dstBytesRemaining, 1);
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

// 		TS_PRINTF("  block end offset  : %lld\n", dstPtr - dstPtrStart);
	}

	// Finalize the file block with a zero which will be read as zero size block by the decompressor
	*reinterpret_cast<SizeType*>(dstPtr) = 0;
	dstPtr += sizeof(SizeType);

	PosType dstBytesWritten = (dstPtr - dstPtrStart);
	dstBuffer.resize(dstBytesWritten);

	return dstBytesWritten;
}

TS_END_PACKAGE2()
