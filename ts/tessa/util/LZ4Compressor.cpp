#include "Precompiled.h"
#include "ts/tessa/util/LZ4Compressor.h"

#include "lz4.h"

#include "ts/tessa/math/CRC.h"

TS_PACKAGE1(util)

LZ4Compressor::LZ4Compressor()
{

}

LZ4Compressor::~LZ4Compressor()
{
// 	finalize();
}
/*
SizeType LZ4Compressor::compressBound(SizeType srcSize)
{
	int bound = LZ4_compressBound(srcSize);
	return bound + OverheadPerBlock;
}

bool LZ4Compressor::initializeCompression()
{
// 	if (mode != Uninitialized)
// 	{
// 		TS_PRINTF("Compressor is already initialized.\n");
// 		return false;
// 	}
// 
// 	_lz4StreamData = LZ4_createStream();
// 	_ringBuffer.resize(CompressionBlockSize * 2 );
// 	_currentRingBufferPosition = 0;
	return true;
}

PosType LZ4Compressor::compressContinue(const char *src, SizeType srcSize, char *dst, SizeType dstSize)
{
	if (_currentRingBufferPosition >= _ringBuffer.size())
		_currentRingBufferPosition = 0;

	return -1;
}

bool LZ4Compressor::initializeDecompression()
{
	return false;
}

PosType LZ4Compressor::decompressContinue(const char *src, SizeType srcSize, char *dst, SizeType dstSize)
{
	return -1;
}

void LZ4Compressor::finalize()
{
	switch (mode)
	{
		case Uninitialized:
			return;

		case StreamCompression:
			LZ4_freeStream((LZ4_stream_t*)_lz4StreamData);
		break;

		case StreamDecompression:
			LZ4_freeStreamDecode((LZ4_streamDecode_t*)_lz4StreamData);
		break;
	}
	
	_lz4StreamData = nullptr;

	_ringBuffer.clear();
	_ringBuffer.shrink_to_fit();
}
*/
PosType LZ4Compressor::fullStreamCompressBound(PosType srcSize)
{
	int bound = LZ4_compressBound((int)srcSize);
	return bound + OverheadPerBlock * (PosType)math::ceil(bound / (float)CompressionBlockSize) + 4;
}

PosType LZ4Compressor::compressFullStream(const char *TS_RESTRICT src, SizeType srcSize, char *TS_RESTRICT dst, SizeType dstSize)
{
	LZ4_stream_t lz4Stream_body;
	LZ4_stream_t *lz4Stream = &lz4Stream_body;
	LZ4_initStream(lz4Stream, sizeof(*lz4Stream));

	const char *TS_RESTRICT srcPtr = src;
	char *TS_RESTRICT dstPtr = dst;

	while (true)
	{
		PosType srcBytesRemaining = srcSize - (srcPtr - src);
		if (srcBytesRemaining <= 0)
			break;

		srcBytesRemaining = math::min(srcBytesRemaining, (PosType)CompressionBlockSize);

		SizeType *blockCompressedSize = reinterpret_cast<SizeType*>(dstPtr);
		dstPtr += sizeof(SizeType);

		SizeType *blockCRC = reinterpret_cast<SizeType*>(dstPtr);
		dstPtr += sizeof(SizeType);

		PosType dstBytesRemaining = dstSize - (dstPtr - dst);
		TS_ASSERT(dstBytesRemaining > 0 && "dst buffer is out of space");

		const int32 compressedBytes = LZ4_compress_fast_continue(lz4Stream, srcPtr, dstPtr, (int32)srcBytesRemaining, (int32)dstBytesRemaining, 1);
		if (compressedBytes <= 0)
		{
			TS_PRINTF("Compression encountered an error.\n");
			return -1;
		}

		*blockCompressedSize = compressedBytes;
		*blockCRC = math::crc32(srcPtr, srcBytesRemaining);

		srcPtr += srcBytesRemaining;
		dstPtr += compressedBytes;
	}

	// Finalize the file block with a zero which will be read as zero size block by the decompressor
	*reinterpret_cast<SizeType*>(dstPtr) = 0;
	dstPtr += sizeof(SizeType);

	PosType dstBytesWritten = (dstPtr - dst);
	return dstBytesWritten;
}

PosType LZ4Compressor::decompressFullStream(const char *TS_RESTRICT srcPtrStart, SizeType srcSize, char *TS_RESTRICT dstPtrStart, SizeType dstSize)
{
	LZ4_streamDecode_t lz4Stream_body;
	LZ4_streamDecode_t *lz4Stream = &lz4Stream_body;
	LZ4_setStreamDecode(lz4Stream, nullptr, 0);

	const char *TS_RESTRICT srcPtr = srcPtrStart;
	char *TS_RESTRICT dstPtr = dstPtrStart;

	while (true)
	{
		const SizeType blockCompressedSize = *reinterpret_cast<const SizeType*>(srcPtr);
		if (blockCompressedSize == 0) // If block size is 0 end was reached
			break;

		srcPtr += sizeof(SizeType);

		const SizeType blockCRC = *reinterpret_cast<const SizeType*>(srcPtr);
		srcPtr += sizeof(SizeType);

		PosType dstBytesRemaining = dstSize - (dstPtr - dstPtrStart);
		TS_ASSERT(dstBytesRemaining > 0 && "dst buffer is out of space");

		const int32 decompressedBytes = LZ4_decompress_safe_continue(lz4Stream, srcPtr, dstPtr, (int32)blockCompressedSize, (int32)dstBytesRemaining);
		if (decompressedBytes <= 0)
		{
			TS_PRINTF("Decompression encountered an error.\n");
			return -1;
		}

		SizeType decompressedCRC = math::crc32(dstPtr, decompressedBytes);
		if (blockCRC != decompressedCRC)
		{
			TS_PRINTF("CRC mismatch: uncompressed data does not match the checksum.\n");
			return -1;
		}

		srcPtr += blockCompressedSize;
		dstPtr += decompressedBytes;
	}

	PosType dstBytesWritten = (dstPtr - dstPtrStart);
	return dstBytesWritten;
}

TS_END_PACKAGE1()
