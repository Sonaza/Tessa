#pragma once

#include <string>

TS_PACKAGE1(file)

namespace priv
{

enum OutputFileModeBits
{
	Out_ModeWrite    = (1 << 0),
	Out_ModeBinary   = (1 << 1),
	Out_ModeTruncate = (1 << 2),
	Out_ModeAppend   = (1 << 3),
};

}

enum OutputFileMode
{
	// Open file for writing without truncating, write position is at the beginning.
	OutputFileMode_Write               = priv::Out_ModeWrite,
	// Open file for writing truncating existing contents.
	OutputFileMode_WriteTruncate       = OutputFileMode_Write | priv::Out_ModeTruncate,
	// Open file for writing, without truncating, write position is at the end.
	OutputFileMode_WriteAppend         = OutputFileMode_Write | priv::Out_ModeAppend,

	// Open file for writing in binary without truncating, write position is at the beginning.
	OutputFileMode_WriteBinary = OutputFileMode_Write | priv::Out_ModeBinary,
	// Open file for writing in binary truncating existing contents.
	OutputFileMode_WriteBinaryTruncate = OutputFileMode_WriteBinary | priv::Out_ModeTruncate,
	// Open file for writing in binary without truncating, write position is at the end.
	OutputFileMode_WriteBinaryAppend   = OutputFileMode_WriteBinary | priv::Out_ModeAppend,
};

class OutputFile : public lang::Noncopyable
{
public:
	OutputFile();
	OutputFile(const String &filepath, OutputFileMode mode);
	~OutputFile();

	// Move constructor and assignment
	OutputFile(OutputFile &&other);
	OutputFile &operator=(OutputFile &&other);
	
	/* Opens file for writing, according to mode.
	 * Returns: full file size in bytes, or -1 if failure or bad.
	 */
	bool open(const String &filepath, OutputFileMode mode);

	/* Closes opened file, flushing the write buffer and also clearing flags. 
	*/
	void close();
	
	/* Writes size bytes from the inBuffer to the write buffer.
	 * Input buffer must be allocated and have at least size bytes of space.
	 * Returns: true if buffering the write succeeded (does not guarantee successful write on disk, use flush and check its return value if required).
	 */
	bool write(const char *inBuffer, BigSizeType size);
	bool write(const unsigned char *inBuffer, BigSizeType size);

	/* Writes the variable value to the write buffer.
	 * In practice, reinterpret casts the value to byte pointer and calls write() on it.
	 * Returns: true if buffering the write succeeded (does not guarantee successful write on disk, use flush and check its return value if required).
	 */
	template <class Type>
	bool writeVariable(const Type &value);

	bool writeString(const char *str);
	bool writeString(const std::string &str);
	bool writeString(const String &str);
	
	/* Sets file position to given position relative to beginning
	 * Returns: new position, or -1 if failure or bad.
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

	/* Clears error flags.
	*/
	void clearFlags();
	
	/* Returns: true if file is open and writable.
	 */
	operator bool() const;
	
	/* Returns: true if file is not open or unwritable.
	*/
	bool operator!() const;

private:
	void *filePtr = nullptr;
	mutable bool bad = false;
};

template <>
TS_FORCEINLINE bool OutputFile::writeVariable<std::string>(const std::string &value)
{
	return write(value.c_str(), value.size());
}

template <>
TS_FORCEINLINE bool OutputFile::writeVariable<String>(const String &value)
{
	const std::basic_string<uint8> utf8str = value.toUtf8();
	return write(utf8str.c_str(), utf8str.size());
}

template <class Type>
bool OutputFile::writeVariable(const Type &value)
{
	static_assert(std::is_trivially_copyable<Type>::value, "Only trivially copyable types can be directly written.");
	return write(reinterpret_cast<const char*>(&value), sizeof(Type));
}

TS_END_PACKAGE1()

