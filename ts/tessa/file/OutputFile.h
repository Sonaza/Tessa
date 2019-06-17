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
	
	/* Opens file for writing, according to mode.
	 * Returns: full file size in bytes, or -1 if failure or bad.
	 */
	bool open(const std::string &filepath, OutputFileMode mode);

	/* Closes opened file, flushing the write buffer and also clearing flags. 
	*/
	void close();
	
	/* Writes size bytes on the inBuffer.
	 * Buffer must be allocated and have at least size bytes of space.
	 * Returns: true if buffering the write succeeded (does not guarantee successful write on disk, use flush and check its return value if required).
	 */
	bool write(const char *inBuffer, BigSizeType size);
	
	/* Sets file position to given position.
	 * Returns: new position, or -1 if failure or bad.
	 */
	PosType seek(PosType pos);
	
	/* Returns: current file position, or -1 if failure or bad.
	 */
	PosType tell() const;
	
	/* Flushes the internal stream, writing current buffer on the disk.
	 * Returns: true if flush was successful, false if failure or bad.
	 */
	bool flush();
	
	/* Returns: true if file is open and writable.
	 */
	bool isOpen() const;
	
	/* Returns: true if a failure has been encountered and the internal stream is bad.
	 */
	bool isBad() const;
	
	/* Returns: true if file is open and writable.
	 */
	operator bool() const;
	
	/* Returns: true if file is not open or unwritable.
	*/
	bool operator!() const;

private:
	mutable std::mutex mutex;
	void *_filePtr = nullptr;
	mutable bool _bad = false;
};

TS_END_PACKAGE1()

