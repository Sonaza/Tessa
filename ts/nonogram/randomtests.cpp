#include "Precompiled.h"

using namespace ts;

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

#if 0

#include "lz4.h"

enum
{
	COMPRESS_BLOCK_SIZE = 1024 * 64,
};

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

#include "ts/tessa/file/FileList.h"
#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/file/InputFile.h"
#include "ts/tessa/file/OutputFile.h"

#include "ts/tessa/file/ArchivistWriter.h"
#include "ts/tessa/file/ArchivistReader.h"
#include "ts/tessa/file/ArchivistReaderExtractor.h"
#include "ts/tessa/util/LZ4Compressor.h"

class Streamy : public sf::InputStream
{
	file::ArchivistReaderExtractor extractor;

public:
	Streamy(file::ArchivistReader &reader, const std::string &file)
	{
		if (!reader.getFileExtractor(file, extractor))
		{
			TS_ASSERT(!"Failed to get extractor");
		}
	}

	~Streamy()
	{
		extractor.close();
	}

	virtual Int64 read(void* data, Int64 size)
	{
		PosType bytesread = extractor.read((char*)data, size);
// 		TS_PRINTF("read %d bytes (bytes read %d)\n", size, bytesread);
		return bytesread;
	}

	virtual Int64 seek(Int64 position)
	{
		PosType seeked = extractor.seek(position);
// 		TS_PRINTF("seek %d (seeked %d)\n", position, seeked);
		return seeked;
	}

	virtual Int64 tell()
	{
// 		TS_PRINTF("tell %d\n", extractor.tell());
		return extractor.tell();
	}

	virtual Int64 getSize()
	{
// 		TS_PRINTF("size %d\n", extractor.getSize());
		return extractor.getSize();
	}
};

#include "lz4.h"
#include "ts/tessa/math/CRC.h"

bool writeImageRaw(int mode)
{
	sf::Image vertImage;
	vertImage.loadFromFile("archivist/vert.png");

	BigSizeType pixelsize = vertImage.getSize().x * vertImage.getSize().y * 4;

	PosType bound = util::LZ4Compressor::fullStreamCompressBound(pixelsize);
	std::vector<char> dstBuffer(bound);

	const char *srcBuffer = (const char*)vertImage.getPixelsPtr();

	file::OutputFile vert("archivist/vert.rgba", file::OutputFileMode_WriteBinaryTruncate);

	vert.writeVariable(vertImage.getSize().x);
	vert.writeVariable(vertImage.getSize().y);

	switch (mode)
	{
		case 0:
		{
			vert.write(srcBuffer, pixelsize);
		}
		break;

		case 1:
		{
			util::LZ4Compressor cmp;
			PosType compressSize = cmp.compressFullStream(srcBuffer, (SizeType)pixelsize, &dstBuffer[0], (SizeType)bound);
			vert.write(&dstBuffer[0], compressSize);
		}
		break;

		case 2:
		{
			int compressSize = LZ4_compress_default(srcBuffer, &dstBuffer[0], (SizeType)pixelsize, (SizeType)bound);
			vert.writeVariable(compressSize);
			vert.write(&dstBuffer[0], compressSize);
		}
		break;
	}

	vert.close();

	return true;
}

bool loadImageRaw(const std::string &filepath, sf::Texture &texture)
{
	file::InputFile input(filepath, file::InputFileMode_ReadBinary);

	SizeType width = 0;
	SizeType height = 0;
	input.readVariable(width);
	input.readVariable(height);
	PosType pixelsize = width * height * 4;

	std::vector<char> dstBuffer(pixelsize);
	PosType bytesRead = input.read(&dstBuffer[0], pixelsize);
	TS_ASSERT(bytesRead == pixelsize);

	sf::Image image;
	image.create(width, height, (sf::Uint8*)&dstBuffer[0]);

	return texture.loadFromImage(image);
}

PosType loadImageRawCompressed(const std::string &filepath, sf::Texture &texture)
{
	file::InputFile input(filepath, file::InputFileMode_ReadBinary);

	SizeType width = 0;
	SizeType height = 0;
	input.readVariable(width);
	input.readVariable(height);
	PosType pixelsize = width * height * 4;

	int blockSize;
	input.readVariable(blockSize);

	std::vector<char> srcBuffer(blockSize);
	std::vector<char> dstBuffer(pixelsize);

	PosType bytesRead = input.read(&srcBuffer[0], blockSize);
	TS_ASSERT(bytesRead == blockSize);

	int decompressSize = LZ4_decompress_safe(&srcBuffer[0], &dstBuffer[0], (SizeType)bytesRead, (SizeType)pixelsize);
	TS_ASSERT(decompressSize == pixelsize);

	sf::Image image;
	image.create(width, height, (sf::Uint8*)&dstBuffer[0]);

	return texture.loadFromImage(image) ? 0 : -1;
}

PosType loadImageRawStreamingCompressed(const std::string &filepath, sf::Texture &texture)
{
	file::InputFile input(filepath, file::InputFileMode_ReadBinary);

	SizeType width = 0;
	SizeType height = 0;
	input.readVariable(width);
	input.readVariable(height);
	PosType pixelsize = width * height * 4;

	std::vector<char> dstBuffer(pixelsize);

	LZ4_streamDecode_t lz4Stream_body;
	LZ4_streamDecode_t *lz4Stream = &lz4Stream_body;
	LZ4_setStreamDecode(lz4Stream, nullptr, 0);

	char *TS_RESTRICT dstPtrStart = &dstBuffer[0];
	char *TS_RESTRICT dstPtr = dstPtrStart;

	const SizeType blockMaxSize = LZ4_COMPRESSBOUND(file::ArchivistConstants::CompressionBlockSize);
	char inputBuffer[blockMaxSize] = { 0 };

	while (true)
	{
		PosType bytesRead;

		SizeType blockCompressedSize;
		bytesRead = input.readVariable(blockCompressedSize);
		TS_ASSERT(bytesRead == 4);
		if (blockCompressedSize == 0) // If block size is 0 end was reached
			break;

		SizeType blockCRC;
		bytesRead = input.readVariable(blockCRC);
		TS_ASSERT(bytesRead == 4);

		bytesRead = input.read(inputBuffer, blockCompressedSize);
		if (bytesRead < blockCompressedSize)
		{
			TS_LOG_ERROR("Error reading file.\n");
			return -1;
		}

		PosType dstBytesRemaining = pixelsize - (dstPtr - dstPtrStart);
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
	}

	PosType dstBytesWritten = (dstPtr - dstPtrStart);
	TS_ASSERT(dstBytesWritten == pixelsize);

	sf::Image image;
	image.create(width, height, (sf::Uint8*)dstPtrStart);

	return texture.loadFromImage(image) ? 0 : -1;
}

void writePack(const std::string &sourceFolder, const std::string &packfile, file::ArchivistCompressionMode mode)
{
	file::FileList list;
	list.open(sourceFolder, true, file::FileListStyle_Files);

	file::ArchivistWriter archiveWriter;
// 	archiveWriter.stageFile("test/49561770_p0.png", "49561770_p0.png", file::CompressionType_LZ4Compression);

	file::FileEntryList files = list.getFullListing();
	TS_PRINTF("%u files\n", files.size());

	for (file::FileEntry &f : files)
	{
		archiveWriter.stageFile(f.getFullFilepath(), f.getFilepath(), mode);
	}
	{
		auto start = std::chrono::system_clock::now();

		archiveWriter.saveToFile(packfile);

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		TS_PRINTF("Packing elapsed time: %uus (%ums)\n", elapsed.count(), elapsed.count() / 1000);
	}
}

void randomtests()
{
// 	if (wtf2())
// 		return;

	std::string packfile = "archivist/amazing.tspack";
	writePack("test/", packfile, file::CompressionType_LZ4FullBlock);

	file::ArchivistReader archiveReader;
	archiveReader.openArchive(packfile);

	auto archivelist = archiveReader.getFileList();

	{
		auto start = std::chrono::system_clock::now();

		for (std::string &file : archivelist)
		{
// 	 		TS_PRINTF("%s (size %0.1f KB)\n", file, archiveReader.getFileSize(file) / 1024.f);
			archiveReader.extractToFile(file, file::utils::joinPaths("archivist/ext2/", file));
		}

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		TS_PRINTF("Extract elapsed time: %uus (%ums)\n", elapsed.count(), elapsed.count() / 1000);
	}

	if (rand() >= 0)
		return;

	sf::RenderWindow window;
	window.create(sf::VideoMode(1680, 1050), "Test");
	window.clear(sf::Color::White);
	window.display();

	bool kakke = true;
	while(kakke)
	{
		sf::Event e;
		while (window.pollEvent(e))
		{
			switch (e.type)
			{
				case sf::Event::KeyPressed:
					kakke = false;
				break;
			}
		}
	}
	window.clear(sf::Color::Red);
	window.display();


	writeImageRaw(0);

	const SizeType loops = 100;
	sf::Texture textures[loops];

	auto start = std::chrono::system_clock::now();

	for (SizeType i = 0; i < loops; ++i)
	{
// 		textures[i].loadFromFile("archivist/vert.png");
		loadImageRaw("archivist/vert.rgba", textures[i]);
// 		loadImageRawStreamingCompressed("archivist/vert.rgba", textures[i]);
// 		loadImageRawCompressed("archivist/vert.rgba", textures[i]);
	}

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	TS_PRINTF("Load elapsed time: %uus (%ums)\n", elapsed.count(), elapsed.count() / 1000);
	TS_PRINTF("  Per load %ums\n", elapsed.count() / 1000 / loops);

	sf::Texture texture;
	sf::Sprite sprite;
// 	Streamy strm(archiveReader, "69103464_p0.png");
// 	texture.loadFromStream(strm);
	sprite.setTexture(textures[0]);

	while(window.isOpen())
	{
		sf::Event e;
		while (window.pollEvent(e))
		{
			switch (e.type)
			{
				case sf::Event::Closed:
				case sf::Event::KeyPressed:
					window.close();
				break;
			}
		}

		window.clear();

		window.draw(sprite);

		window.display();
	}

	return;
}