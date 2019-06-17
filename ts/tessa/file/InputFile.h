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
	
	/* Opens file for reading, according to mode.
	 * Returns: true if file open succeeded. In case of failure the reason is output to the log.
	 */
	bool open(const std::string &filepath, InputFileMode mode);
	
	/* Closes opened file, also clearing flags.
	 */
	void close();

	/* Reads size bytes to the outBuffer.
	 * Buffer must already be allocated and have at least size bytes of space.
	 * Returns: number of bytes read, or 0 on failure or bad.
	 */
	PosType read(char *outBuffer, BigSizeType size);

	/* Sets file position to given position.
	 * Returns: new position, or -1 if failure or bad.
	 */
	PosType seek(PosType pos);
	
	/* Returns: current file position, or -1 if failure or bad.
	 */
	PosType tell() const;
	
	/* Returns: full file size in bytes, or -1 if failure or bad.
	 */
	PosType getFileSize();
	
	/* Returns: true if file is open and readable.
	 */
	bool isOpen() const;

	/* Returns: true if end of file has been reached.
	 */
	bool isEOF() const;

	/* Returns: true if a failure has been encountered and the internal stream is bad.
	 */
	bool isBad() const;
	
	/* Returns: true if file is open and readable.
	 */
	operator bool() const;

	/* Returns: true if file is not open or unreadable.
	*/
	bool operator!() const;

private:
	mutable std::mutex mutex;
	void *_filePtr = nullptr;
	bool _eof = false;
	mutable bool _bad = false;
};

TS_END_PACKAGE1()

