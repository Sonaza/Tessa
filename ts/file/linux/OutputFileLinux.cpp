#include "Precompiled.h"

#if TS_PLATFORM == TS_LINUX

#include "ts/file/OutputFile.h"
#include "ts/file/FileUtils.h"

#include <cstdio>

TS_PACKAGE1(file)

typedef FILE* FileHandle;

OutputFile::OutputFile()
{
}

OutputFile::OutputFile(const String &filepath, OutputFileMode mode, OutputFileCreation creation)
{
	open(filepath, mode, creation);
}

OutputFile::~OutputFile()
{
	close();
}

OutputFile::OutputFile(OutputFile &&other)
{
	*this = std::move(other);
}

OutputFile &OutputFile::operator=(OutputFile &&other)
{
	if (this != &other)
	{
		m_handle = std::exchange(other.m_handle, nullptr);
		m_bad = std::exchange(other.m_bad, false);
	}
	return *this;
}

bool OutputFile::open(const String &filepath, OutputFileMode modeParam, OutputFileCreation creation)
{
	TS_ASSERT(m_handle == nullptr && "OutputFile is already opened.");
	if (m_handle != nullptr)
		return false;

	if (creation == OutputFileCreation_OpenExisting && !exists(filepath))
	{
		TS_LOG_ERROR("Open failed. File does not exist and creation flag prevents creation.\n", filepath);
		return false;
	}

	const char *mode;
	switch (modeParam)
	{
		case OutputFileMode_WriteBinary:         mode = exists(filepath) ? "r+b" : "wb"; break;
		case OutputFileMode_WriteBinaryTruncate: mode = "wb"; break;
		case OutputFileMode_WriteBinaryAppend:   mode = "ab"; break;
		default: TS_ASSERT(!"Unhandled mode"); return false;
	}

	FileHandle file = fopen(filepath.toUtf8().c_str(), mode);
	if (file == nullptr)
	{
		TS_LOG_ERROR("Open failed. File: %s - Error: %s\n", filepath, strerror(errno));
		return false;
	}

	// Not appending so seek back to start
	if ((modeParam & priv::Out_ModeAppend) == 0)
		fseek(file, 0, SEEK_SET);

	m_handle = file;
	return true;
}

void OutputFile::close()
{
	if (m_handle != nullptr)
	{
		FileHandle file = static_cast<FileHandle>(m_handle);
		fclose(file);
	}
	m_handle = nullptr;
	m_bad = false;
}

bool OutputFile::write(const char *inBuffer, uint32 size)
{
	TS_ASSERT(inBuffer != nullptr);

	TS_ASSERT(m_handle != nullptr && "OutputFile is not opened.");
	if (m_handle == nullptr || m_bad == true)
		return false;

	FileHandle file = static_cast<FileHandle>(m_handle);
	uint32 bytesWritten = (uint32)fwrite(inBuffer, sizeof(inBuffer[0]), size, file);
	TS_ASSERT(bytesWritten == size);
	
	if (ferror(file))
	{
		m_bad = true;
		return false;
	}

	return true;
}

bool OutputFile::write(const unsigned char *inBuffer, uint32 size)
{
	return write(reinterpret_cast<const char*>(inBuffer), size);
}

bool OutputFile::writeString(const char *str)
{
	return write(str, std::strlen(str));
}

bool OutputFile::writeString(const std::string &str)
{
	return writeVariable(str);
}

bool OutputFile::writeString(const String &str)
{
	return writeVariable(str);
}

PosType OutputFile::seek(PosType pos)
{
	return seek(pos, SeekFromBeginning);
}

PosType OutputFile::seek(PosType pos, SeekOrigin seekOrigin)
{
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");
	if (m_handle == nullptr || m_bad == true)
		return -1;

	int32 origin;
	switch (seekOrigin)
	{
		default:
		case SeekFromBeginning: origin = SEEK_SET; break;
		case SeekFromCurrent:   origin = SEEK_CUR; break;
		case SeekFromEnd:       origin = SEEK_END; break;
	}

	FileHandle file = static_cast<FileHandle>(m_handle);
	if (fseek(file, (long)pos, origin) == 0)
	{
// 		eof = (feof(file) != 0);
		return ftell(file);
	}
	return -1;
}

PosType OutputFile::tell() const
{
	TS_ASSERT(m_handle != nullptr && "OutputFile is not opened.");
	if (m_handle == nullptr || m_bad == true)
		return -1;

	FileHandle file = static_cast<FileHandle>(m_handle);
	return ftell(file);
}

bool OutputFile::flush()
{
	TS_ASSERT(m_handle != nullptr && "OutputFile is not opened.");
	if (m_handle == nullptr || m_bad == true)
		return false;
	
	FileHandle file = static_cast<FileHandle>(m_handle);
	
	if (fflush(file) == 0)
		return true;

	return false;
}

bool OutputFile::isOpen() const
{
	return m_handle != nullptr && m_bad == false;
}

bool OutputFile::isBad() const
{
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");
	if (m_handle == nullptr || m_bad == true)
		return true;

	FileHandle file = static_cast<FileHandle>(m_handle);
	return ferror(file) != 0;
}

void OutputFile::clearFlags()
{
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");
	if (m_handle == nullptr)
		return;

	FileHandle file = static_cast<FileHandle>(m_handle);
	clearerr(file);
// 	eof = false;
	m_bad = false;
}

OutputFile::operator bool() const
{
	return isOpen();
}

bool OutputFile::operator!() const
{
	return !isOpen();
}

TS_END_PACKAGE1()

#endif
