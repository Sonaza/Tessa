#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "ts/tessa/file/OutputFile.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/tessa/common/IncludeWindows.h"
#include "ts/tessa/common/WindowsUtils.h"

TS_PACKAGE1(file)

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
		std::swap(handle, other.handle);
		std::swap(bad, other.bad);
	}
	return *this;
}

bool OutputFile::open(const String &filepath, OutputFileMode modeParam, OutputFileCreation creation)
{
	TS_ASSERT(handle == nullptr && "OutputFile is already opened.");
	if (handle != nullptr)
		return false;

	bool truncate = (modeParam & priv::Out_ModeTruncate) > 0;
	bool append = (modeParam & priv::Out_ModeAppend) > 0;

	DWORD accessFlags = GENERIC_WRITE;

	if (append)
		accessFlags = FILE_APPEND_DATA;

	DWORD modeFlags = OPEN_EXISTING;
	switch (creation)
	{
		case OutputFileCreation_CreateNew:
		{
			if (truncate)
				modeFlags = CREATE_ALWAYS;
			else
				modeFlags = OPEN_ALWAYS;
		}
		break;

		case OutputFileCreation_OpenExisting:
		{
			if (truncate)
				modeFlags = TRUNCATE_EXISTING;
			else
				modeFlags = OPEN_EXISTING;
		}
		break;
	}

	HANDLE fileHandle = CreateFileW(
		filepath.toWideString().c_str(),
		accessFlags,
		FILE_SHARE_READ,
		nullptr,
		modeFlags,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		TS_WLOG_ERROR("File open failed: %s  File: %s\n", windows::getLastErrorAsString(), filepath);
		return false;
	}

	handle = fileHandle;

	return true;
}

void OutputFile::close()
{
	if (handle != nullptr && handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle((HANDLE)handle);
	}
	handle = nullptr;
	bad = false;
}

bool OutputFile::write(const char *inBuffer, uint32 size)
{
	TS_ASSERT(inBuffer != nullptr);

	TS_ASSERT(handle != nullptr && "OutputFile is not opened.");
	if (handle == nullptr || bad == true)
		return false;

	DWORD numBytesWritten;
	if (WriteFile((HANDLE)handle, inBuffer, size, &numBytesWritten, nullptr) == FALSE)
	{
		TS_WLOG_ERROR("File write failed: %s\n", windows::getLastErrorAsString());
		return false;
	}
	TS_ASSERT(size == numBytesWritten);

	return true;
}

bool OutputFile::write(const unsigned char *inBuffer, uint32 size)
{
	return write(reinterpret_cast<const char*>(inBuffer), size);
}

bool OutputFile::writeString(const char *str)
{
	return write(str, (SizeType)std::strlen(str));
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
	return (PosType)newPos.QuadPart;
}

PosType OutputFile::tell() const
{
	TS_ASSERT(handle != nullptr && "OutputFile is not opened.");
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

bool OutputFile::flush()
{
	TS_ASSERT(handle != nullptr && "OutputFile is not opened.");
	if (handle == nullptr || bad == true)
		return false;
	
	return FlushFileBuffers((HANDLE)handle) == TRUE;
}

bool OutputFile::isOpen() const
{
	return handle != nullptr && bad == false;
}

bool OutputFile::isBad() const
{
	TS_ASSERT(handle != nullptr && "InputFile is not opened.");
	if (handle == nullptr || bad == true)
		return true;

	return false;
}

void OutputFile::clearFlags()
{
	TS_ASSERT(handle != nullptr && "InputFile is not opened.");
	if (handle == nullptr)
		return;

	bad = false;
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
