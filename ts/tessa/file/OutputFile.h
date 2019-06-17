#pragma once

#include <string>
#include <mutex>

TS_PACKAGE1(file)

namespace priv
{

enum OutputFileModeBits
{
	Out_ModeWrite    = 0b0001,
	Out_ModeBinary   = 0b0010,
	Out_ModeTruncate = 0b0100,
	Out_ModeAppend   = 0b1000,
};

}

enum OutputFileMode
{
	// Basic write mode
	OutputFileMode_Write               = priv::Out_ModeWrite,
	// Truncate after open
	OutputFileMode_WriteTruncate       = priv::Out_ModeWrite | priv::Out_ModeTruncate,
	// Sets pointer at end
	OutputFileMode_WriteAppend         = priv::Out_ModeWrite | priv::Out_ModeAppend,
	// Write in binary
	OutputFileMode_WriteBinary         = priv::Out_ModeWrite | priv::Out_ModeBinary,
	// Write in binary, truncate after open
	OutputFileMode_WriteBinaryTruncate = priv::Out_ModeWrite | priv::Out_ModeBinary | priv::Out_ModeTruncate,
	// Write in binary, sets pointer at end
	OutputFileMode_WriteBinaryAppend   = priv::Out_ModeWrite | priv::Out_ModeBinary | priv::Out_ModeAppend,
};

class OutputFile : public lang::Noncopyable
{
public:
	OutputFile();
	OutputFile(const std::string &filepath, OutputFileMode mode);
	~OutputFile();

	// Move constructor and assignment
	OutputFile(OutputFile &&other);
	OutputFile &operator=(OutputFile &&other);

	bool open(const std::string &filepath, OutputFileMode mode);
	void close();

	bool write(const char *inBuffer, BigSizeType size);
	
	void seek(PosType pos);
	PosType tell() const;

	void flush();

	bool isOpen() const;
	bool isBad() const;

	operator bool() const;
	bool operator!() const;

private:
	mutable std::mutex mutex;
	void *_filePtr = nullptr;
	mutable bool _bad = false;
};

TS_END_PACKAGE1()

