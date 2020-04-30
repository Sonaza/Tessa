#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "ts/file/InputFile.h"

#include "ts/lang/common/IncludeWindows.h"
#include "ts/lang/common/WindowsUtils.h"

#include <cstdio>

TS_PACKAGE1(file)

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

	DWORD modeFlags = OPEN_EXISTING;

	HANDLE fileHandle = CreateFileW(
		filepath.toWideString().c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		modeFlags,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		TS_WLOG_ERROR("File open failed: %s  File: %s\n",
			windows::getLastErrorAsString(), filepath);
		return false;
	}

	m_handle = fileHandle;
	return true;
}

void InputFile::close()
{
	if (m_handle != nullptr && m_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle((HANDLE)m_handle);
	}
	m_handle = nullptr;
	m_eof = false;
	m_bad = false;
	m_filesize = -1;
}

PosType InputFile::read(char *outBuffer, uint32 numBytesToRead)
{
	TS_ASSERT(outBuffer != nullptr);
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");

	if (m_handle == nullptr || m_bad == true)
		return -1;

	if (m_eof == true)
		return 0;

	DWORD numBytesRead;
	if (ReadFile((HANDLE)m_handle, outBuffer, numBytesToRead, &numBytesRead, nullptr) == FALSE)
	{
		TS_WLOG_ERROR("File read failed: %s\n", windows::getLastErrorAsString());
		m_bad = true;
		return -1;
	}
	m_eof = (numBytesRead < numBytesToRead);
	return numBytesRead;
}

PosType InputFile::read(unsigned char *outBuffer, uint32 numBytesToRead)
{
	return read(reinterpret_cast<char*>(outBuffer), numBytesToRead);
}

PosType InputFile::readLine(char *outBuffer, uint32 size, const char linebreak)
{
	TS_ASSERT(outBuffer != nullptr);
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");
	TS_ASSERT(size > 0 && "numBytesToRead must be greater than 0.");

	if (m_handle == nullptr || m_bad == true || size == 0)
		return -1;

	if (m_eof == true)
		return 0;

	char c;
	char *ptr = outBuffer;

	PosType numBytesToRead = size;
	while (numBytesToRead-- > 0)
	{
		DWORD numBytesRead;
		if (ReadFile((HANDLE)m_handle, &c, sizeof(c), &numBytesRead, nullptr) == FALSE)
		{
			TS_WLOG_ERROR("File read failed: %s\n", windows::getLastErrorAsString());
			m_bad = true;
			return -1;
		}

		if (numBytesRead == 0)
			break;

		*ptr++ = c;

		if (c == linebreak)
			break;
	}

	*ptr = '\0';
	return (ptr - outBuffer);
}

PosType InputFile::readLine(unsigned char *outBuffer, uint32 numBytesToRead, const char linebreak)
{
	return readLine(reinterpret_cast<char*>(outBuffer), numBytesToRead, linebreak);
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

	DWORD method;
	switch (seekOrigin)
	{
		default:
		case SeekFromBeginning: method = FILE_BEGIN; break;
		case SeekFromCurrent:   method = FILE_CURRENT; break;
		case SeekFromEnd:       method = FILE_END; break;
	}
	
	LARGE_INTEGER dist;
	dist.QuadPart = pos;

	LARGE_INTEGER newPos;
	if (SetFilePointerEx((HANDLE)m_handle, dist, &newPos, method) == FALSE)
	{
		TS_WLOG_ERROR("Seek failed: %s\n", windows::getLastErrorAsString());
		return -1;
	}
	// Assume not eof anymore, any reads will re-set flag if file is still eof.
	m_eof = false;
	return (PosType)newPos.QuadPart;
}

PosType InputFile::tell() const
{
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");
	if (m_handle == nullptr || m_bad == true)
		return -1;

	// Zero distance move with FILE_CURRENT returns current position in file.
	LARGE_INTEGER dist;
	dist.QuadPart = 0;

	LARGE_INTEGER pos;
	if (SetFilePointerEx((HANDLE)m_handle, dist, &pos, FILE_CURRENT) == FALSE)
	{
		TS_WLOG_ERROR("Tell failed: %s\n", windows::getLastErrorAsString());
		return -1;
	}
	return (PosType)pos.QuadPart;
}

PosType InputFile::getSize()
{
	TS_ASSERT(m_handle != nullptr && "InputFile is not opened.");
	if (m_handle == nullptr || m_bad == true)
		return -1;

	// Return cached file size
	if (m_filesize != -1)
		return m_filesize;
	
	LARGE_INTEGER size;
	if (GetFileSizeEx((HANDLE)m_handle, &size) == FALSE)
	{
		TS_WLOG_ERROR("getSize failed: %s\n", windows::getLastErrorAsString());
		return -1;
	}
	return (PosType)size.QuadPart;
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
