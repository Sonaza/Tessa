#include "Precompiled.h"
#include "ts/tessa/file/ArchivistReaderExtractor.h"

#include "lz4.h"

#include "ts/tessa/math/CRC.h"

TS_PACKAGE1(file)

ArchivistReaderExtractor::ArchivistReaderExtractor()
{

}

ArchivistReaderExtractor::~ArchivistReaderExtractor()
{
	close();
}

void ArchivistReaderExtractor::close()
{
	if (initialized == false)
		return;

	initialized = false;
	eof = false;
	decompressionComplete = false;
	memset(&header, 0, sizeof(header));

	blockOffsets.clear();
	blockOffsets.shrink_to_fit();
	decompressedBuffer.clear();
	decompressedBuffer.shrink_to_fit();

	numDecompressedBlocks = 0;
	currentPosition = 0;

	archiveFile.close();

	if (streamDecode != nullptr)
		LZ4_freeStreamDecode((LZ4_streamDecode_t*)streamDecode);
	streamDecode = nullptr;
}

PosType ArchivistReaderExtractor::read(char *outBuffer, BigSizeType size)
{
	TS_ASSERT(initialized);
	
	switch (header.compression)
	{
	case CompressionType_NoCompression:
		return readNoCompressed(outBuffer, size);

	case CompressionType_LZ4FullBlock:
		return readLZ4FullBlockCompressed(outBuffer, size);

	case CompressionType_LZ4Streaming:
		return readLZ4StreamingCompressed(outBuffer, size);
	}

	return -1;
}

PosType ArchivistReaderExtractor::seek(PosType pos)
{
	TS_ASSERT(initialized);

	eof = false;
	currentPosition = pos;
	PosType bytesRemaining = header.filesize - currentPosition;
	if (bytesRemaining < 0)
	{
		eof = true;
		currentPosition = header.filesize;
	}
	return currentPosition;
}

PosType ArchivistReaderExtractor::tell() const
{
	TS_ASSERT(initialized);
	return currentPosition;
}

SizeType ArchivistReaderExtractor::getSize() const
{
	TS_ASSERT(initialized);
	return header.filesize;
}

bool ArchivistReaderExtractor::isEOF() const
{
	return eof;
}

PosType ArchivistReaderExtractor::readNoCompressed(char *outBuffer, BigSizeType size)
{
	if (eof)
		return 0;

	PosType position = header.offset + currentPosition;
	PosType bytesRemaining = header.filesize - currentPosition;
	TS_ASSERT(bytesRemaining >= 0 && "read overflow");

	BigSizeType numBytesToRead = math::min((BigSizeType)bytesRemaining, size);

	archiveFile.seek(position);
	PosType bytesRead = archiveFile.read(outBuffer, numBytesToRead);
	if (bytesRead < (PosType)numBytesToRead)
	{
		TS_LOG_ERROR("Was not able to read the requested amount.\n");
		return -1;
	}
	if (bytesRead < (PosType)size)
		eof = true;

	currentPosition += bytesRead;
	return bytesRead;
}

PosType ArchivistReaderExtractor::readLZ4FullBlockCompressed(char *outBuffer, BigSizeType size)
{
	if (eof)
		return 0;

	PosType bytesRemaining = header.filesize - currentPosition;
	TS_ASSERT(bytesRemaining >= 0 && "read overflow");

	BigSizeType numBytesToRead = math::min((BigSizeType)bytesRemaining, size);
	if (numBytesToRead == 0)
		return 0;

	if (decompressionComplete == false)
	{
		if (decompressFullBlock() < 0)
		{
			TS_LOG_ERROR("Decompressing full block failed.\n");
			return -1;
		}
	}
	
	char *srcPtr = &decompressedBuffer[currentPosition];
	memcpy(outBuffer, srcPtr, numBytesToRead);

	currentPosition += numBytesToRead;
	if (currentPosition >= header.filesize)
		eof = true;

	return numBytesToRead;
}

PosType ArchivistReaderExtractor::decompressFullBlock()
{
	TS_ASSERT(decompressionComplete == false);

	archiveFile.seek(header.offset);

	PosType bytesRead;

	SizeType blockCompressedSize;
	bytesRead = archiveFile.readVariable(blockCompressedSize);
	TS_ASSERT(bytesRead == 4);
	if (blockCompressedSize == 0)
		return -1;

	SizeType blockCRC;
	bytesRead = archiveFile.readVariable(blockCRC);
	TS_ASSERT(bytesRead == 4);

	std::vector<char> srcBuffer(blockCompressedSize);
	bytesRead = archiveFile.read(&srcBuffer[0], blockCompressedSize);
	if (bytesRead < blockCompressedSize)
	{
		TS_LOG_ERROR("Error reading file.\n");
		return -1;
	}

	decompressedBuffer.resize(header.filesize);

	const Int32 bytesDecompressed = LZ4_decompress_safe(&srcBuffer[0], &decompressedBuffer[0], blockCompressedSize, header.filesize);
	if (bytesDecompressed < (PosType)header.filesize)
	{
		TS_LOG_ERROR("Decompression encountered an error, full file was not decompressed.\n");
		decompressedBuffer.clear();
		decompressedBuffer.shrink_to_fit();
		return -1;
	}

	const SizeType decompressedCRC = math::crc32(&decompressedBuffer[0], header.filesize);
	if (blockCRC != decompressedCRC)
	{
		TS_LOG_ERROR("CRC mismatch: uncompressed data does not match the checksum.\n");
		return -1;
	}

	decompressionComplete = true;

	return (PosType)bytesDecompressed;
}

PosType ArchivistReaderExtractor::readLZ4StreamingCompressed(char *outBuffer, BigSizeType size)
{
	if (eof)
		return 0;
	
	PosType bytesRemaining = header.filesize - currentPosition;
	TS_ASSERT(bytesRemaining >= 0 && "read overflow");

	BigSizeType numBytesToRead = math::min((BigSizeType)bytesRemaining, size);
	if (numBytesToRead == 0)
		return 0;

	PosType bufferedBytesRemaining = decompressedBuffer.size() - currentPosition;
	if (bufferedBytesRemaining < (PosType)numBytesToRead)
	{
		PosType overflowBytes = math::abs(bufferedBytesRemaining - (PosType)numBytesToRead);
		SizeType numBlocksToDecompress =  (SizeType)math::ceil(overflowBytes / (float)ArchivistConstants::CompressionBlockSize);
		
		PosType bytesDecompressed = decompressStreamingBlocks(numBlocksToDecompress);
		if (bytesDecompressed < overflowBytes)
		{
			TS_LOG_ERROR("Was not able to decompress enough bytes.\n");
			return -1;
		}
	}

	char *srcPtr = &decompressedBuffer[currentPosition];
	memcpy(outBuffer, srcPtr, numBytesToRead);

	currentPosition += numBytesToRead;
	if (currentPosition >= header.filesize)
		eof = true;

	return numBytesToRead;
}

PosType ArchivistReaderExtractor::decompressStreamingBlocks(SizeType numBlocksToDecompress)
{
	TS_ASSERT(decompressionComplete == false);

	PosType bytesDecompressed = numDecompressedBlocks * ArchivistConstants::CompressionBlockSize;
	PosType bytesToDecompress = numBlocksToDecompress * ArchivistConstants::CompressionBlockSize;

	PosType bufferSize = bytesDecompressed + bytesToDecompress;
	bufferSize = math::min((PosType)header.filesize, bufferSize);
	decompressedBuffer.resize(bufferSize);

	char *TS_RESTRICT dstPtrStart = &decompressedBuffer[bytesDecompressed];
	char *TS_RESTRICT dstPtr = dstPtrStart;

	const SizeType dictSize = math::min<SizeType>(ArchivistConstants::DictionarySize, (SizeType)bytesDecompressed);
	char *TS_RESTRICT dictStart = dstPtrStart - dictSize;

	LZ4_streamDecode_t *lz4Stream = (LZ4_streamDecode_t*)streamDecode;
	LZ4_setStreamDecode(lz4Stream, dictStart, dictSize);

	const SizeType blockMaxSize = LZ4_COMPRESSBOUND(ArchivistConstants::CompressionBlockSize);
	char inputBuffer[blockMaxSize] = { 0 };

	PosType fileStartOffset = blockOffsets[numDecompressedBlocks];
	archiveFile.seek(fileStartOffset);

	for (SizeType index = 0; index < numBlocksToDecompress; ++index)
	{
		PosType bytesRead;

		SizeType blockCompressedSize;
		bytesRead = archiveFile.readVariable(blockCompressedSize);
		TS_ASSERT(bytesRead == 4);
		if (blockCompressedSize == 0) // If block size is 0 end was reached
		{
			decompressionComplete = true;
			LZ4_freeStreamDecode((LZ4_streamDecode_t*)streamDecode);
			streamDecode = nullptr;
			break;
		}

		TS_ASSERT(blockCompressedSize <= blockMaxSize);

		SizeType blockCRC;
		bytesRead = archiveFile.readVariable(blockCRC);
		TS_ASSERT(bytesRead == 4);

		bytesRead = archiveFile.read(inputBuffer, blockCompressedSize);
		if (bytesRead < blockCompressedSize)
		{
			TS_LOG_ERROR("Error reading file.\n");
			return -1;
		}

		PosType dstBytesRemaining = bytesToDecompress - (dstPtr - dstPtrStart);
		TS_ASSERT(dstBytesRemaining > 0 && "dst buffer is out of space");

		const Int32 decompressedBytes = LZ4_decompress_safe_continue(lz4Stream, inputBuffer, dstPtr, (Int32)blockCompressedSize, (Int32)dstBytesRemaining);
		if (decompressedBytes <= 0)
		{
			TS_LOG_ERROR("Decompression encountered an error.\n");
			return -1;
		}

		SizeType decompressedCRC = math::crc32(dstPtr, decompressedBytes);
		if (blockCRC != decompressedCRC)
		{
			TS_LOG_ERROR("CRC mismatch: uncompressed data does not match the checksum.\n");
			return -1;
		}

		dstPtr += decompressedBytes;
		numDecompressedBlocks++;
	}

	PosType dstBytesWritten = (dstPtr - dstPtrStart);
	return dstBytesWritten;
}

bool ArchivistReaderExtractor::initialize(const ArchivistFileHeader &headerParam, const std::string &archiveFilepath)
{
	TS_ASSERT(!initialized);

	if (!archiveFile.open(archiveFilepath, InputFileMode_ReadBinary))
	{
		TS_ASSERT(false);
		return false;
	}

	header = headerParam;

	switch (header.compression)
	{
		case CompressionType_NoCompression:
		{
			// Nothing special here
		}
		break;

		case CompressionType_LZ4FullBlock:
		{
			// Nothing special here
		}
		break;

		case CompressionType_LZ4Streaming:
		{
			PosType offset = header.offset;
			blockOffsets.push_back(offset);

			while (true)
			{
				archiveFile.seek(offset);

				SizeType blockSize;
				PosType bytesRead = archiveFile.readVariable(blockSize);
				if (bytesRead < 4)
				{
					TS_LOG_ERROR("Unexpected read error or end of file.\n");
					return false;
				}
				TS_ASSERT(blockSize < LZ4_COMPRESSBOUND(ArchivistConstants::CompressionBlockSize));
				if (blockSize == 0)
					break;

				offset += blockSize + sizeof(SizeType) * 2; // Account for block size and CRC

				blockOffsets.push_back(offset);
			}

			streamDecode = LZ4_createStreamDecode();
		}
		break;
	}

	initialized = true;

	return true;
}

TS_END_PACKAGE1()
