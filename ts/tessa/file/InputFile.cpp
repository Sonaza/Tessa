#include "Precompiled.h"
#include "ts/tessa/file/InputFile.h"

#include <fstream>

TS_PACKAGE1(file)

typedef std::ifstream InputFileStream;

InputFile::InputFile()
{
}

InputFile::InputFile(const std::string &filepath, InputFileMode mode)
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
		_filePtr = other._filePtr;
		_eof = other._eof;
		_bad = other._bad;
		_filesize = other._filesize;

		other._filePtr = nullptr;
		other._eof = false;
		other._bad = false;
		other._filesize = -1;
	}
	return *this;
}

bool InputFile::open(const std::string &filepath, InputFileMode mode)
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr == nullptr && "InputFile is already opened.");
	if (_filePtr != nullptr)
		return false;

	InputFileStream *file = new InputFileStream;
	if (file == nullptr)
		return false;

	Int32 modeBits = std::ios_base::in;
	if ((mode & priv::InputFileModeBits::In_ModeBinary) > 0)
		modeBits |= std::ios_base::binary;

	file->open(filepath.c_str(), modeBits);
	if (!(*file))
	{
		TS_LOG_ERROR("Open failed. File: %s - Error: %s\n", filepath, strerror(errno));
		delete file;
		return false;
	}

	_filePtr = file;
	return true;
}

void InputFile::close()
{
	std::lock_guard<std::mutex> mg(mutex);
	if (_filePtr != nullptr)
	{
		InputFileStream *file = static_cast<InputFileStream*>(_filePtr);
		file->close();
		delete file;
	}
	_filePtr = nullptr;
	_eof = false;
	_bad = false;
	_filesize = -1;
}

PosType InputFile::read(char *outBuffer, BigSizeType size)
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr != nullptr && "InputFile is not opened.");
	if (_filePtr == nullptr || _bad == true || _eof == true)
		return 0;

	InputFileStream *file = static_cast<InputFileStream*>(_filePtr);
	file->read(outBuffer, size);
	if (!file->bad())
	{
		PosType bytesRead = file->gcount();
		if (file->eof())
			_eof = true;
		return bytesRead;
	}

	_bad = true;
	return 0;
}

PosType InputFile::seek(PosType pos)
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr != nullptr && "InputFile is not opened.");
	if (_filePtr == nullptr || _bad == true)
		return -1;

	InputFileStream *file = static_cast<InputFileStream*>(_filePtr);
	if (!file->seekg(pos))
	{
		_bad = true;
		return -1;
	}
	_eof = file->eof();
	return pos;
}

PosType InputFile::tell() const
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr != nullptr && "InputFile is not opened.");
	if (_filePtr == nullptr || _bad == true)
		return -1;

	InputFileStream *file = static_cast<InputFileStream*>(_filePtr);
	PosType pos = file->tellg();
	if (!(*file))
	{
		_bad = true;
		return -1;
	}
	return pos;
}

PosType InputFile::getFileSize()
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr != nullptr && "InputFile is not opened.");
	if (_filePtr == nullptr || _bad == true)
		return -1;

	// Return cached file size
	if (_filesize != -1)
		return _filesize;

	InputFileStream *file = static_cast<InputFileStream*>(_filePtr);
	
	BigSizeType pos = file->tellg();
	if (file->seekg(0, std::ios_base::end))
	{
		BigSizeType size = file->tellg();
		if (*file)
		{
			if (!file->seekg(pos))
			{
				_bad = true;
				return -1;
			}
			// Store cached file size so don't need to do this again
			_filesize = size;
			return size;
		}
	}

	_bad = true;
	return -1;
}

bool InputFile::isOpen() const
{
	std::lock_guard<std::mutex> mg(mutex);
	if (_filePtr != nullptr && _bad == false)
	{
		InputFileStream *file = static_cast<InputFileStream*>(_filePtr);
		return file->is_open() && (*file);
	}
	return false;
}

bool InputFile::isEOF() const
{
	std::lock_guard<std::mutex> mg(mutex);
	return _eof;
}

bool InputFile::isBad() const
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr != nullptr && "InputFile is not opened.");
	if (_filePtr == nullptr || _bad == true)
		return true;

	InputFileStream *file = static_cast<InputFileStream*>(_filePtr);
	return file->bad();
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
