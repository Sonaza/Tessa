#include "Precompiled.h"
#include "ts/tessa/file/InputFile.h"

#include "ts/tessa/common/IncludeWindows.h"
#include "ts/tessa/common/WindowsUtils.h"

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
		handle = std::exchange(other.handle, nullptr);
		eof = std::exchange(other.eof, false);
		bad = std::exchange(other.bad, false);
		filesize = std::exchange(other.filesize, -1);
	}
	return *this;
}

bool InputFile::open(const String &filepath, InputFileMode modeParam)
{
	TS_ASSERT(handle == nullptr && "InputFile is already opened.");
	if (handle != nullptr)
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

	handle = fileHandle;
	return true;
}

void InputFile::close()
{
	if (handle != nullptr && handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle((HANDLE)handle);
	}
	handle = nullptr;
	eof = false;
	bad = false;
	filesize = -1;
}

PosType InputFile::read(char *outBuffer, uint32 numBytesToRead)
{
	TS_ASSERT(outBuffer != nullptr);
	TS_ASSERT(handle != nullptr && "InputFile is not opened.");

	if (handle == nullptr || bad == true)
		return -1;

	if (eof == true)
		return 0;

	DWORD numBytesRead;
	if (ReadFile((HANDLE)handle, outBuffer, numBytesToRead, &numBytesRead, nullptr) == FALSE)
	{
		TS_WLOG_ERROR("File read failed: %s\n", windows::getLastErrorAsString());
		bad = true;
		return -1;
	}
	eof = (numBytesRead < numBytesToRead);
	return numBytesRead;
}

PosType InputFile::read(unsigned char *outBuffer, uint32 numBytesToRead)
{
	return read(reinterpret_cast<char*>(outBuffer), numBytesToRead);
}

PosType InputFile::readLine(char *outBuffer, uint32 size, const char linebreak)
{
	TS_ASSERT(outBuffer != nullptr);
	TS_ASSERT(handle != nullptr && "InputFile is not opened.");
	TS_ASSERT(size > 0 && "numBytesToRead must be greater than 0.");

	if (handle == nullptr || bad == true || size == 0)
		return -1;

	if (eof == true)
		return 0;

	char c;
	char *ptr = outBuffer;

	PosType numBytesToRead = size;
	while (numBytesToRead-- > 0)
	{
		DWORD numBytesRead;
		if (ReadFile((HANDLE)handle, &c, sizeof(c), &numBytesRead, nullptr) == FALSE)
		{
			TS_WLOG_ERROR("File read failed: %s\n", windows::getLastErrorAsString());
			bad = true;
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
	TS_ASSERT(handle != nullptr && "InputFile is not opened.");
	if (handle == nullptr || bad == true)
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
	if (SetFilePointerEx((HANDLE)handle, dist, &newPos, method) == FALSE)
	{
		TS_WLOG_ERROR("Seek failed: %s\n", windows::getLastErrorAsString());
		return -1;
	}
	// Assume not eof anymore, any reads will re-set flag if file is still eof.
	eof = false;
	return (PosType)newPos.QuadPart;
}

PosType InputFile::tell() const
{
	TS_ASSERT(handle != nullptr && "InputFile is not opened.");
	if (handle == nullptr || bad == true)
		return -1;

	// Zero distance move with FILE_CURRENT returns current position in file.
	LARGE_INTEGER dist;
	dist.QuadPart = 0;

	LARGE_INTEGER pos;
	if (SetFilePointerEx((HANDLE)handle, dist, &pos, FILE_CURRENT) == FALSE)
	{
		TS_WLOG_ERROR("Tell failed: %s\n", windows::getLastErrorAsString());
		return -1;
	}
	return (PosType)pos.QuadPart;
}

PosType InputFile::getSize()
{
	TS_ASSERT(handle != nullptr && "InputFile is not opened.");
	if (handle == nullptr || bad == true)
		return -1;

	// Return cached file size
	if (filesize != -1)
		return filesize;
	
	LARGE_INTEGER size;
	if (GetFileSizeEx((HANDLE)handle, &size) == FALSE)
	{
		TS_WLOG_ERROR("getSize failed: %s\n", windows::getLastErrorAsString());
		return -1;
	}
	return (PosType)size.QuadPart;
}

bool InputFile::isOpen() const
{
	return handle != nullptr && bad == false;
}

bool InputFile::isEOF() const
{
	return eof;
}

bool InputFile::isBad() const
{
	TS_ASSERT(handle != nullptr && "InputFile is not opened.");
	return handle == nullptr || bad == true;
}

void InputFile::clearFlags()
{
	TS_ASSERT(handle != nullptr && "InputFile is not opened.");
	if (handle == nullptr)
		return;

	eof = false;
	bad = false;
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
