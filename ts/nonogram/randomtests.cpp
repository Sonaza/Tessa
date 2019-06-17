#include "Precompiled.h"

#include "ts/tessa/system/ConfigReader.h"

#if 0

class Base
{
public:
	virtual void doThing() = 0;
};

class Derived : public Base
{
public:
	virtual void doThing()
	{
		TS_PRINTF("Doing a thing! Number %d\n", number);
	}
	ts::Int32 number = 1337;
};

class Poopoo
{
public:
	virtual void doSomtinElse()
	{
		TS_PRINTF("Doing a somtin else! Number %d\n", number);
	}
	ts::Int32 number;
};

class CustomDeleter
{
public:
	void operator()(void *ptr)
	{
		TS_PRINTF("Using custom deleter!\n");
		free(ptr);
	}
};

int pointer_test()
{
	int *customRawPtr = (int*)malloc(sizeof(int));
	*customRawPtr = 1234512345;
	ts::UniquePointer<int, CustomDeleter> customAD(customRawPtr);

	customAD.reset();

	// 	make_shared

	// std::shared_ptr<int> potato(new int(1234));

	ts::ScopedPointer<std::string> strptr(new std::string("Potato text"));

	TS_PRINTF("String! %s\n", *strptr);

	ts::SharedPointer<Base> basePtr(new Derived, CustomDeleter());
	ts::SharedPointer<Derived> derivedPtr = ts::staticPointerCast<Derived>(basePtr);

	std::unique_ptr<int> stdunique = std::make_unique<int>(1241325);
	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(-1, stdunique);

	if (basePtr == derivedPtr)
	{
		TS_PRINTF("They are pointing to the same thing!\n");
	}
	else
	{
		TS_PRINTF("They are not pointing to the same thing!\n");
	}

	basePtr.reset();

	ts::UniquePointer<int> scp1 = ts::makeUnique<int>(1337);

	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(-1, scp1);

	int *rawPtr = scp1.dismiss();

	if (scp1)
	{
		TS_PRINTF("POINTER 1 IS A THING!\n");
	}
	else
	{
		TS_PRINTF("POINTER 1 IS NOT A THING!\n");
	}

	TS_PRINTF("rawPtr on the other hand has value %d\n", *rawPtr);
	delete rawPtr;
	rawPtr = nullptr;

	ts::UniquePointer<int> scp2 = std::move(scp1);

	TS_VERIFY_POINTERS_WITH_RETURN_VALUE(-2, scp1, scp2);

	// 	*scp1 = 1753474;

	TS_PRINTF("shared pointer 1 value %d\n", *scp1);
	TS_PRINTF("shared pointer 2 value %d\n", *scp2);

	scp1.reset();
	TS_PRINTF("Reset shared pointer 1\n");

	if (scp1)
	{
		TS_PRINTF("POINTER 1 IS A THING!\n");
	}
	else
	{
		TS_PRINTF("POINTER 1 IS NOT A THING!\n");
	}

	TS_PRINTF("shared pointer 2 value %d\n", *scp2);

	return 1337;
}

#endif 

#if 0

#pragma warning(disable : 4189)
#define ALIGNED(alignment, type, variable) __declspec(align(alignment)) type variable
#include <cstdlib>
#include <emmintrin.h>

float randomfloat(float min, float max)
{
	return (rand() / (float)RAND_MAX) * (max - min) + min;
}

int sse_test()
{
	ALIGNED(16, float, div[4]) = { 2.f, 3.f, 4.f, 5.f };
	__m128 *divsse = (__m128*)div;

	{
		std::vector<float> values;
		values.resize(4 * 500000);
		for (float &v : values)
		{
			v = randomfloat(50.f, 1337.f);
		}

		auto start = std::chrono::system_clock::now();

		for (size_t i = 0; i < values.size(); i += 4)
		{
			__m128 *m = (__m128 *)&values[i];
			_mm_store_ps((float*)&values[i], _mm_div_ps(*m, *divsse));
		}

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		TS_PRINTF("SSE elapsed: %uus (%ums)\n", elapsed.count(), elapsed.count() / 1000);
	}

	{
		std::vector<float> values;
		values.resize(4 * 500000);
		for (float &v : values)
		{
			v = randomfloat(50.f, 1337.f);
		}

		auto start = std::chrono::system_clock::now();

		for (size_t i = 0; i < values.size(); ++i)
		{
			values[i] = values[i] / div[i % 4];
		}

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		TS_PRINTF("Normal elapsed: %uus (%ums)\n", elapsed.count(), elapsed.count() / 1000);
	}

	return 0;
}

#endif

#if 1

#include "lz4.h"
#include <fstream>

/*
int lz4test()
{
	std::ifstream input;
	input.open("lz4/file.c", std::ios_base::in | std::ios_base::binary);

	std::string src_data(
		(std::istreambuf_iterator<char>(input)),
		std::istreambuf_iterator<char>()
		);
	TS_PRINTF("File size %u bytes\n", src_data.size());

	int src_size = (int)src_data.size() + 1;
	int dst_capacity = LZ4_compressBound(src_size);
	char *compress_dst = (char*)malloc(dst_capacity);
	memset(compress_dst, 0, dst_capacity);

	int compress_result = LZ4_compress_default(&src_data[0], compress_dst, src_size, dst_capacity);
	if (compress_result > 0)
	{
		TS_PRINTF("Compression was success! String compressed %d => %d bytes\n", src_data.size(), compress_result);

		std::ofstream output;
		output.open("lz4/output.lz4", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
		output.write(compress_dst, compress_result);
		output.close();
	}
	else
	{
		TS_PRINTF("Compression failed.\n");
	}

	free(compress_dst);

	return 0;
}*/

enum
{
	COMPRESS_BLOCK_SIZE = 1024 * 64,
};

#include "ts/tessa/file/InputFile.h"
#include "ts/tessa/file/OutputFile.h"
#include "ts/tessa/math/CRC.h"

void lz4_streaming_compress()
{
	ts::file::InputFile input;
	input.open("lz4/file.c", ts::file::InputFileMode_ReadBinary);

	ts::file::OutputFile output;
	output.open("lz4/compressed.lz4", ts::file::OutputFileMode_WriteBinaryTruncate);

	char inputBuffer[2][COMPRESS_BLOCK_SIZE];
	size_t bufferIndex = 0;
	
	const size_t compressBufferSize = LZ4_COMPRESSBOUND(COMPRESS_BLOCK_SIZE);
	char compressBuffer[compressBufferSize];

	LZ4_stream_t lz4Stream_body;
	LZ4_stream_t *lz4Stream = &lz4Stream_body;
	LZ4_initStream(lz4Stream, sizeof(*lz4Stream));

	size_t readBytesTotal = 0;
	size_t compressedBytesTotal = 0;
	size_t blockCounter = 0;
	while (true)
	{
		char *const inputBufferPtr = inputBuffer[bufferIndex];
// 		memset(inputBufferPtr, 0, BlockSize);

		size_t bytesRead = input.read(inputBufferPtr, COMPRESS_BLOCK_SIZE);
		if (bytesRead == 0)
		{
			TS_PRINTF("No more bytes to read.\n");
			break;
		}

// 		memset(compressBuffer, 0, sizeof(compressBuffer));
		const int compressedBytes = LZ4_compress_fast_continue(lz4Stream, inputBufferPtr, compressBuffer, (int)bytesRead, sizeof(compressBuffer), 1);
		if (compressedBytes <= 0)
		{
			TS_PRINTF("Compression encountered an error.\n");
			break;
		}

		output.write(reinterpret_cast<const char*>(&compressedBytes), sizeof(compressedBytes));

		uint32_t crc = ts::math::crc32((const ts::Uint8*)inputBufferPtr, (size_t)bytesRead);
		output.write(reinterpret_cast<const char*>(&crc), sizeof(crc));
		
		output.write(compressBuffer, (size_t)compressedBytes);
		
		TS_PRINTF("Block %u:\n", blockCounter);
		TS_PRINTF("  %u bytes compressed to %d\n", bytesRead, compressedBytes);
		TS_PRINTF("  Data CRC32 0x%08llX\n", crc);

		readBytesTotal += bytesRead;
		compressedBytesTotal += compressedBytes + 4;

		bufferIndex = (bufferIndex + 1) % 2;
		blockCounter++;
	}

	int endnull = 0;
	output.write(reinterpret_cast<const char*>(&endnull), sizeof(endnull));

	output.flush();

	TS_PRINTF("Compression complete!\n");
	TS_PRINTF("  %u bytes => %u bytes (ratio %0.2f)\n\n",
		readBytesTotal, compressedBytesTotal,
		compressedBytesTotal / (float)readBytesTotal
	);

	input.close();
	output.close();
}

void lz4_streaming_uncompress()
{
	ts::file::InputFile input;
	input.open("lz4/compressed.lz4", ts::file::InputFileMode_ReadBinary);

	ts::file::OutputFile output;
	output.open("lz4/uncompressed.c", ts::file::OutputFileMode_WriteBinaryTruncate);

	const size_t decodeBufferPages = 2;
	char inputBuffer[decodeBufferPages][COMPRESS_BLOCK_SIZE];
	size_t bufferIndex = 0;

	const size_t compressBufferSize = LZ4_COMPRESSBOUND(COMPRESS_BLOCK_SIZE);
	char compressBuffer[compressBufferSize];

	LZ4_streamDecode_t lz4Stream_body;
	LZ4_streamDecode_t *lz4Stream = &lz4Stream_body;
	LZ4_setStreamDecode(lz4Stream, nullptr, 0);

	size_t blockCounter = 0;
	while (true)
	{
		size_t bytesRead = 0;

		int compressedBytes = 0;
		bytesRead = input.read((char*)&compressedBytes, sizeof(compressedBytes));
		if (bytesRead < 4 || compressedBytes <= 0)
		{
			TS_PRINTF("Unable to read number of compressed bytes: end of file or error.\n");
			break;
		}

		uint32_t crc = 0;
		bytesRead = input.read((char*)&crc, sizeof(crc));
		if (bytesRead < 4)
		{
			TS_PRINTF("Unable to read CRC.\n");
			break;
		}

		char *const inputBufferPtr = inputBuffer[bufferIndex];

		bytesRead = input.read(inputBufferPtr, compressedBytes);
		if (bytesRead != (size_t)compressedBytes)
		{
			TS_PRINTF("Number of read bytes doesn't match number of compressed bytes.\n");
			break;
		}

		const int decompressedBytes = LZ4_decompress_safe_continue(lz4Stream, inputBufferPtr, compressBuffer, compressedBytes, sizeof(compressBuffer));
		if (decompressedBytes <= 0)
		{
			TS_PRINTF("Decompression encountered an error.\n");
			break;
		}

		TS_PRINTF("Block %u:\n", blockCounter);

		uint32_t decompressedCRC = ts::math::crc32((const ts::Uint8*)compressBuffer, (size_t)decompressedBytes);
		TS_PRINTF("  Original     CRC32 0x%08llX\n", crc);
		TS_PRINTF("  Decompressed CRC32 0x%08llX\n", decompressedCRC);
		if (crc != decompressedCRC)
		{
			TS_PRINTF("  Uncompressed data CRC mismatch\n");
			break;
		}

		TS_PRINTF("  %d bytes uncompressed to %d\n", compressedBytes, decompressedBytes);

		output.write(compressBuffer, decompressedBytes);

// 		std::string block_marker = TS_FMT("===BLOCK %u END===\n", blockCounter);
// 		output.write(block_marker.c_str(), block_marker.size());

		bufferIndex = (bufferIndex + 1) % decodeBufferPages;
		blockCounter++;

		if (input.isEOF())
		{
			TS_PRINTF("End of file!\n");
			break;
		}
	}

	output.flush();

	input.close();
	output.close();

	return;
}

#endif

void randomtests()
{
	ts::file::InputFile input;
	bool asd = input.open("kakkapeppu", ts::file::InputFileMode_ReadBinary);
	TS_PRINTF("%d\n", asd);

// 	lz4_streaming_compress();
// 	lz4_streaming_uncompress();
}