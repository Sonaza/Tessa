#include "Precompiled.h"
#include "ts/tessa/file/ArchivistReader.h"

#include "ts/tessa/file/ArchivistReaderExtractor.h"
#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/math/CRC.h"
#include "ts/tessa/math/Hash.h"

TS_PACKAGE1(file)

namespace
{

const SizeType ArchivistReaderSupportedFormatVersion = 1;

}

ArchivistReader::ArchivistReader()
{

}

bool ArchivistReader::openArchive(const std::string &filepath)
{
// 	std::lock_guard<std::mutex> mg(mutex);

	TS_ASSERT(archive.isOpen() == false && "Archive is already opened.");

	archiveFilepath = filepath;
	if (!archive.open(archiveFilepath, InputFileMode_ReadBinary))
	{
		TS_LOG_ERROR("Unable to open archive file. File: %s", archiveFilepath);
		return false;
	}

	ArchivistFileFormat format;
	archive.read(reinterpret_cast<char*>(&format), sizeof(format));

	if (strcmp(format.formatString, TS_ARCHIVIST_FORMAT_STRING) != 0)
	{
		TS_LOG_ERROR("Archive format string does not match expected value.\n");
		return false;
	}

	if (format.version != ArchivistReaderSupportedFormatVersion)
	{
		TS_LOG_ERROR("Archive file format is unsupported. Archive version: %d. Supported: %d\n",
			format.version, ArchivistReaderSupportedFormatVersion);
		return false;
	}

	SizeType numFiles = 0;
	archive.readVariable(numFiles);
	if (numFiles == 0)
	{
		TS_LOG_ERROR("Archive has zero files?\n");
		return false;
	}

	headers.clear();
	for (SizeType index = 0; index < numFiles; ++index)
	{
		ArchivistFileHeader header;
		PosType bytesRead = archive.readVariable(header);
		if (bytesRead < sizeof(ArchivistFileHeader))
		{
			TS_LOG_ERROR("Failed to read header index %u.\n", index);
			return false;
		}

		const SizeType hash = math::simpleHash32(std::string(header.filename));
		headers.emplace(hash, header);
	}

	return true;
}

bool ArchivistReader::fileExists(const std::string &filename) const
{
	const SizeType hash = math::simpleHash32(filename);
	return headers.count(hash) > 0;
}

PosType ArchivistReader::getFileSize(const std::string &filename)
{
	SizeType hash = math::simpleHash32(filename);
	return headers.count(hash) > 0 ? headers[hash].filesize : -1;
}

std::vector<std::string> ArchivistReader::getFileList() const
{
	std::vector<std::string> filelist;
	ArchivistFileHeaders::const_iterator it = headers.begin();
	for (; it != headers.end(); ++it)
	{
		filelist.push_back(it->second.filename);
	}
	return filelist;
}

SizeType ArchivistReader::getNumFiles() const
{
	return (SizeType)headers.size();
}

bool ArchivistReader::extractToFile(const std::string &filename, const std::string &targetFilepath)
{
	ArchivistReaderExtractor extractor;
	if (!getFileExtractor(filename, extractor))
	{
		TS_ASSERT(false);
		return false;
	}

	OutputFile output(targetFilepath, OutputFileMode_WriteBinaryTruncate);

	char buffer[1024 * 64];
	while(true)
	{
		PosType bytesRead = extractor.read(buffer, sizeof(buffer));
		TS_ASSERT(bytesRead >= 0);
		if (bytesRead <= 0)
			break;

		output.write(buffer, bytesRead);
	}
	output.close();

	return true;
}

bool ArchivistReader::getFileExtractor(const std::string &filename, ArchivistReaderExtractor &extractor)
{
	const SizeType hash = math::simpleHash32(filename);
	if (headers.count(hash) == 0)
	{
		TS_LOG_ERROR("Archive does not contain the request file. File: %s\n", filename);
		return false;
	}

	return extractor.initialize(headers[hash], archiveFilepath);
}

TS_END_PACKAGE1()
