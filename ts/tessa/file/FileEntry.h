#pragma once

TS_PACKAGE1(file)

class FileEntry
{
public:
	FileEntry() = default;
	FileEntry(FileEntry &&other);
	FileEntry &operator=(FileEntry &&other);

	const String &getFilepath() const;
	const String getFullFilepath() const;

	bool isDirectory() const;
	bool isFile() const;

private:
	friend class FileList;
	String filepath;
	String rootDirectory;
	bool isDir = false;
};

TS_END_PACKAGE1()

