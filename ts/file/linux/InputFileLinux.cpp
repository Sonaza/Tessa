#include "Precompiled.h"

#if TS_PLATFORM == TS_LINUX

#include "ts/file/InputFile.h"
#include <cstdio>

TS_PACKAGE1(file)

typedef FILE FileHandle;

InputFile::InputFile()
{
}

InputFile::InputFile(const String &filepath, InputFileMode mode)
{
	open(filepath, mode);
}

InputFile::~InputFile()
{
	close();
}

InputFile::InputFile(InputFile &&other)
{
	*this = std::move(other);
}

InputFile &InputFile::operator=(InputFile &&other)
{
	if (this != &other)
	{
		m_handle = std::exchange(other.m_handle, nullptr);
		m_eof = std::exchange(other.m_eof, false);
		m_bad = std::exchange(other.m_bad, false);
		m_filesize = std::exchange(other.m_filesize, -1);
	}
	return *this;
}

bool InputFile::open(const String &filepath, InputFileMode modeParam)
{
	TS_ASSERT(m_handle == nullptr && "InputFile is already opened.");
	if (m_handle != nullptr)
		return false;

	const char *mode;
	switch (modeParam)
	{
		case InputFileMode_Read:       mode = "r";  break;
		case InputFileMode_ReadBinary: mode = "rb"; break;
		default: TS_ASSERT(!"Unhandled mode"); return false;
	}

	FileHandle *file = fopen(filepath.toUtf8().c_str(), mode);
	if (file == nullptr)
	{
		TS_LOG_ERROR("Open failed. File: %s - Error: %s\n", filepath, strerror(errno));
		return false;
	}

	m_handle = file;
	return true;
}

void InputFile::close()
{
	if (m_handle != nullptr)
	{
		FileHandle *file = static_cast<FileHandle*>(m_handle);
		fclose(file);
	}
	m_handle = nullptr;
	m_eof = false;
	m_bad = false;
	m_filesize = -1;
}

PosType InputFile::read(char *outBuffer, uint32 size)
{
	TS_ASSERT(outBuffer != nullptr);
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");

	if (m_handle == nullptr || m_bad == true)
		return -1;

	if (m_eof == true)
		return 0;

	FileHandle *file = static_cast<FileHandle*>(m_handle);

	PosType numBytesRead = (PosType)fread(outBuffer, sizeof(outBuffer[0]), size, file);
	if (ferror(file))
	{
		m_bad = true;
		return -1;
	}
	
	m_eof = (feof(file) != 0);

	return numBytesRead;
}

PosType InputFile::read(unsigned char *outBuffer, uint32 size)
{
	return read(reinterpret_cast<char*>(outBuffer), size);
}

PosType InputFile::readLine(char *outBuffer, uint32 size, const char linebreak)
{
	TS_ASSERT(outBuffer != nullptr);
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");
	TS_ASSERT(size > 0 && "Size must be greater than 0.");

	if (m_handle == nullptr || m_bad == true || size == 0)
		return -1;

	if (m_eof == true)
		return 0;

	char c;
	char *ptr = outBuffer;

	FileHandle *file = static_cast<FileHandle*>(m_handle);

	PosType bytesToRead = (PosType)size;
	while (bytesToRead-- > 0)
	{
		if (fread(&c, sizeof(char), 1, file) == 0)
			break;

		*ptr++ = c;

		if (feof(file) != 0 || c == linebreak)
			break;
	}

	*ptr = '\0';
	return (ptr - outBuffer);
}

PosType InputFile::readLine(unsigned char *outBuffer, uint32 size, const char linebreak)
{
	return readLine(reinterpret_cast<char*>(outBuffer), size, linebreak);
}

PosType InputFile::seek(PosType pos)
{
	return seek(pos, SeekFromBeginning);
}

PosType InputFile::seek(PosType pos, SeekOrigin seekOrigin)
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

	FileHandle *file = static_cast<FileHandle*>(m_handle);
	if (fseek(file, (long)pos, origin) == 0)
	{
		m_eof = (feof(file) != 0);
		return ftell(file);
	}

	return -1;
}

PosType InputFile::tell() const
{
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");
	if (m_handle == nullptr || m_bad == true)
		return -1;

	FileHandle *file = static_cast<FileHandle*>(m_handle);
	return ftell(file);
}

PosType InputFile::getSize()
{
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");
	if (m_handle == nullptr || m_bad == true)
		return -1;

	// Return cached file size
	if (m_filesize != -1)
		return m_filesize;
	
	BigSizeType originalPosition = tell();

	if (seek(0, SeekFromEnd) > 0)
	{
		BigSizeType size = tell();
		seek(originalPosition, SeekFromBeginning);
		return size;
	}

// 	bad = true;
	return -1;
}

bool InputFile::isOpen() const
{
	return m_handle != nullptr && m_bad == false;
}

bool InputFile::isEOF() const
{
	return m_eof;
}

bool InputFile::isBad() const
{
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");
	return m_handle == nullptr || m_bad == true;
}

void InputFile::clearFlags()
{
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");
	if (m_handle == nullptr)
		return;

	FileHandle *file = static_cast<FileHandle*>(m_handle);
	clearerr(file);
	m_eof = false;
	m_bad = false;
}

InputFile::operator bool() const
{
	return isOpen();
}

bool InputFile::operator!() const
{
	return !isOpen();
}

TS_END_PACKAGE1()

#endif
