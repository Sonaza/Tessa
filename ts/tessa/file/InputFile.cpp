#include "Precompiled.h"
#include "ts/tessa/file/InputFile.h"

#include <fstream>

#include <cstdlib>

TS_PACKAGE1(file)

typedef FILE InputFileStream;

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
		std::swap(filePtr, other.filePtr);
		std::swap(eof, other.eof);
		std::swap(bad, other.bad);
		std::swap(filesize, other.filesize);
	}
	return *this;
}

bool InputFile::open(const String &filepath, InputFileMode modeParam)
{
	TS_ASSERT(filePtr == nullptr && "InputFile is already opened.");
	if (filePtr != nullptr)
		return false;

	String mode;
	switch (modeParam)
	{
		case InputFileMode_Read:       mode = "r";  break;
		case InputFileMode_ReadBinary: mode = "rb"; break;
		default: TS_ASSERT(!"Unhandled mode"); return false;
	}

#if TS_PLATFORM == TS_WINDOWS
	InputFileStream *file = _wfopen(filepath.toWideString().c_str(), mode.toWideString().c_str());
#else
	InputFileStream *file = fopen(filepath.toAnsiString().c_str(), mode.toAnsiString().c_str());
#endif
	if (file == nullptr)
	{
		TS_LOG_ERROR("Open failed. File: %s - Error: %s\n", filepath, strerror(errno));
		return false;
	}

	filePtr = file;
	return true;
}

void InputFile::close()
{
	if (filePtr != nullptr)
	{
		InputFileStream *file = static_cast<InputFileStream*>(filePtr);
		fclose(file);
	}
	filePtr = nullptr;
	eof = false;
	bad = false;
	filesize = -1;
}

PosType InputFile::read(char *outBuffer, BigSizeType size)
{
	TS_ASSERT(outBuffer != nullptr);
	TS_ASSERT(filePtr != nullptr && "InputFile is not opened.");

	if (filePtr == nullptr || bad == true)
		return -1;

	if (eof == true)
		return 0;

	InputFileStream *file = static_cast<InputFileStream*>(filePtr);

	PosType numBytesRead = fread(outBuffer, sizeof(decltype(*outBuffer)), size, file);

	if (ferror(file))
	{
		bad = true;
		return -1;
	}
	
	eof = (feof(file) != 0);

	return numBytesRead;
}

PosType InputFile::read(unsigned char *outBuffer, BigSizeType size)
{
	return read(reinterpret_cast<char*>(outBuffer), size);
}

PosType InputFile::seek(PosType pos)
{
	return seek(pos, SeekFromBeginning);
}

PosType InputFile::seek(PosType pos, SeekOrigin seekOrigin)
{
	TS_ASSERT(filePtr != nullptr && "InputFile is not opened.");
	if (filePtr == nullptr || bad == true)
		return -1;

	int32 origin;
	switch (seekOrigin)
	{
		default:
		case SeekFromBeginning: origin = SEEK_SET; break;
		case SeekFromCurrent:   origin = SEEK_CUR; break;
		case SeekFromEnd:       origin = SEEK_END; break;
	}

	InputFileStream *file = static_cast<InputFileStream*>(filePtr);
	if (fseek(file, (long)pos, origin) == 0)
	{
		eof = (feof(file) != 0);
		return ftell(file);
	}

	return -1;
}

PosType InputFile::tell() const
{
	TS_ASSERT(filePtr != nullptr && "InputFile is not opened.");
	if (filePtr == nullptr || bad == true)
		return -1;

	InputFileStream *file = static_cast<InputFileStream*>(filePtr);
	return ftell(file);
}

PosType InputFile::getSize()
{
	TS_ASSERT(filePtr != nullptr && "InputFile is not opened.");
	if (filePtr == nullptr || bad == true)
		return -1;

	// Return cached file size
	if (filesize != -1)
		return filesize;
	
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
	return filePtr != nullptr && bad == false;
}

bool InputFile::isEOF() const
{
	return eof;
}

bool InputFile::isBad() const
{
	TS_ASSERT(filePtr != nullptr && "InputFile is not opened.");
	return filePtr == nullptr || bad == true;
}

void InputFile::clearFlags()
{
	TS_ASSERT(filePtr != nullptr && "InputFile is not opened.");
	if (filePtr == nullptr)
		return;

	InputFileStream *file = static_cast<InputFileStream*>(filePtr);
	clearerr(file);
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
