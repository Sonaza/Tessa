#pragma once

#include <string>
#include <mutex>

TS_PACKAGE1(file)

namespace priv
{

enum InputFileModeBits
{
	In_ModeRead   = 0b0001,
	In_ModeBinary = 0b0010,
};

}

enum InputFileMode
{
	// Read file in text mode
	InputFileMode_Read       = priv::In_ModeRead,
	// Read file in binary mode
	InputFileMode_ReadBinary = priv::In_ModeRead | priv::In_ModeBinary,
};

class InputFile : public lang::Noncopyable
{
public:
	InputFile();
	InputFile(const std::string &filepath, InputFileMode mode);
	~InputFile();

	// Move constructor and assignment
	InputFile(InputFile &&other);
	InputFile &operator=(InputFile &&other);

	bool open(const std::string &filepath, InputFileMode mode);
	void close();

	PosType read(char *outBuffer, BigSizeType size);
	
	void seek(PosType pos);
	PosType tell() const;
	
	BigSizeType getFileSize();

	bool isOpen() const;
	bool isEOF() const;
	bool isBad() const;

	operator bool() const;
	bool operator!() const;

private:
	mutable std::mutex mutex;
	void *_filePtr = nullptr;
	bool _eof = false;
	mutable bool _bad = false;
};

TS_END_PACKAGE1()

