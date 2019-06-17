#include "Precompiled.h"
#include "ts/tessa/file/OutputFile.h"

#include <fstream>

TS_PACKAGE1(file)

typedef std::ofstream OutputFileStream;

OutputFile::OutputFile()
{
}

OutputFile::OutputFile(const std::string &filepath, OutputFileMode mode)
{
	open(filepath, mode);
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
		_filePtr = other._filePtr;
		_bad = other._bad;

		other._filePtr = nullptr;
		other._bad = false;
	}
	return *this;
}

bool OutputFile::open(const std::string &filepath, OutputFileMode mode)
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr == nullptr && "OutputFile is already opened.");
	if (_filePtr != nullptr)
		return false;

	OutputFileStream *file = new OutputFileStream;
	if (file == nullptr)
		return false;

	Int32 modeBits = std::ios_base::out;
	if ((mode & priv::Out_ModeBinary) > 0)
		modeBits |= std::ios_base::binary;
	if ((mode & priv::Out_ModeTruncate) > 0)
		modeBits |= std::ios_base::trunc;
	if ((mode & priv::Out_ModeAppend) > 0)
		modeBits |= std::ios_base::ate;

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

void OutputFile::close()
{
	std::lock_guard<std::mutex> mg(mutex);
	if (_filePtr != nullptr)
	{
		OutputFileStream *file = static_cast<OutputFileStream*>(_filePtr);
		file->close();
		delete file;
	}
	_filePtr = nullptr;
	_bad = false;
}

bool OutputFile::write(const char *inBuffer, BigSizeType size)
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr != nullptr && "OutputFile is not opened.");
	if (_filePtr == nullptr || _bad == true)
		return false;

	OutputFileStream *file = static_cast<OutputFileStream*>(_filePtr);
	file->write(inBuffer, size);
	if (!file->bad())
		return true;

	_bad = true;
	return false;
}

void OutputFile::seek(PosType pos)
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr != nullptr && "OutputFile is not opened.");
	if (_filePtr == nullptr || _bad == true)
		return;

	OutputFileStream *file = static_cast<OutputFileStream*>(_filePtr);
	if (!file->seekp(pos))
		_bad = true;
}

PosType OutputFile::tell() const
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr != nullptr && "OutputFile is not opened.");
	if (_filePtr == nullptr || _bad == true)
		return 0;

	OutputFileStream *file = static_cast<OutputFileStream*>(_filePtr);
	BigSizeType pos = file->tellp();
	if (!(*file))
		_bad = true;
	return pos;
}

void OutputFile::flush()
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr != nullptr && "OutputFile is not opened.");
	if (_filePtr == nullptr || _bad == true)
		return;
	
	OutputFileStream *file = static_cast<OutputFileStream*>(_filePtr);
	if (!file->flush())
		_bad = true;
}

bool OutputFile::isOpen() const
{
	std::lock_guard<std::mutex> mg(mutex);
	if (_filePtr != nullptr && _bad == false)
	{
		OutputFileStream *file = static_cast<OutputFileStream*>(_filePtr);
		return file->is_open() && (*file);
	}
	return false;
}

bool OutputFile::isBad() const
{
	std::lock_guard<std::mutex> mg(mutex);
	TS_ASSERT(_filePtr != nullptr && "InputFile is not opened.");
	if (_filePtr == nullptr || _bad == true)
		return true;

	OutputFileStream *file = static_cast<OutputFileStream*>(_filePtr);
	return file->bad();
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
