#pragma once

#include <string>

TS_PACKAGE1(file)

namespace priv
{

enum InputFileModeBits
{
	In_ModeRead   = (1 << 0),
	In_ModeBinary = (1 << 1),
};

}

enum InputFileMode
{
	// Read file in binary mode
	InputFileMode_ReadBinary = priv::In_ModeRead | priv::In_ModeBinary,
};

class InputFile : public lang::Noncopyable
{
public:
	InputFile();
	InputFile(const String &filepath, InputFileMode mode);
	~InputFile();

	// Move constructor and assignment
	InputFile(InputFile &&other);
	InputFile &operator=(InputFile &&other);
	
	/* Opens file for reading, according to mode.
	 * Returns: true if file open succeeded. In case of failure the reason is output to the log.
	 */
	bool open(const String &filepath, InputFileMode mode);
	
	/* Closes opened file, also clearing flags.
	 */
	void close();

	/* Reads size bytes to the outBuffer.
	 * Buffer must already be allocated and have at least size bytes of space.
	 * Returns one of these:
	 *    number of bytes read (may be less than bytes requested when reaching end of file),
	 *    0 if already end of file,
     *    or -1 on failure or bad.
	 */
	PosType read(char *outBuffer, uint32_t numBytesToRead);
	PosType read(unsigned char *outBuffer, uint32_t numBytesToRead);

	PosType readLine(char *outBuffer, uint32_t numBytesToRead, const char linebreak = '\n');
	PosType readLine(unsigned char *outBuffer, uint32_t numBytesToRead, const char linebreak = '\n');

	/* Reads variable directly.
	 */
	template <class Type>
	PosType readVariable(Type &value);

	/* Sets file position to given position (uses beginning origin).
	 * Returns: new position, or -1 if failure/bad.
	 */
	PosType seek(PosType pos);

	/* Sets file position to given position relative to the seek origin.
	 * Returns: new position, or -1 if failure/bad.
	 */
	enum SeekOrigin
	{
		SeekFromBeginning = 0,
		SeekFromCurrent   = 1,
		SeekFromEnd       = 2,
	};
	PosType seek(PosType pos, SeekOrigin seekOrigin);
	
	/* Returns: current file position, or -1 if failure or bad.
	 */
	PosType tell() const;
	
	/* Returns: full file size in bytes, or -1 if failure or bad.
	 */
	PosType getSize();
	
	/* Returns: true if file is open and readable.
	 */
	bool isOpen() const;

	/* Returns: true if end of file has been reached.
	 */
	bool isEOF() const;

	/* Returns: true if a failure has been encountered and the internal stream is bad.
	 */
	bool isBad() const;

	/* Clears eof and error flags.
	 */
	void clearFlags();
	
	/* Returns: true if file is open and readable.
	 */
	operator bool() const;

	/* Returns: true if file is not open or unreadable.
	*/
	bool operator!() const;

private:
	void *m_handle = nullptr;
	bool m_eof = false;
	mutable bool m_bad = false;
	PosType m_filesize = -1;
};

template <class Type>
PosType InputFile::readVariable(Type &value)
{
	static_assert(std::is_trivially_copyable<Type>::value, "Only trivially copyable types can be directly read.");
	return read(reinterpret_cast<char*>(&value), sizeof(Type));
}

TS_END_PACKAGE1()

