#pragma once

TS_PACKAGE1(util)

class LZ4Compressor
{
public:
	LZ4Compressor();
	~LZ4Compressor();

// 	bool compressBlock(const char *src, SizeType srcSize, char *dst, SizeType dstSize);

// 	static SizeType compressBound(SizeType srcSize);
// 	
// 	bool initializeCompression();
// 	PosType compressContinue(const char *src, SizeType srcSize, char *dst, SizeType dstSize);
// 
// 	bool initializeDecompression();
// 	PosType decompressContinue(const char *src, SizeType srcSize, char *dst, SizeType dstSize);
// 	
// 	void finalize();

	static PosType fullStreamCompressBound(PosType srcSize);
	PosType compressFullStream(const char *TS_RESTRICT src, SizeType srcSize, char *TS_RESTRICT dst, SizeType dstSize);
	PosType decompressFullStream(const char *TS_RESTRICT src, SizeType srcSize, char *TS_RESTRICT dst, SizeType dstSize);


private:
	enum {
		CompressionBlockSize = 1024 * 64,
		OverheadPerBlock = 8,
	};

	enum CurrentMode
	{
		Uninitialized,
		StreamCompression,
		StreamDecompression,
	};
	void *_lz4StreamData = nullptr;
	SizeType _currentRingBufferPosition = 0;
	std::vector<char> _ringBuffer;
	CurrentMode mode = Uninitialized;
};

TS_END_PACKAGE1()

