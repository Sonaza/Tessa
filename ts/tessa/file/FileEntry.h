#pragma once

#include <string>

TS_PACKAGE1(file)

class FileEntry
{
public:
	FileEntry() = default;
	FileEntry(FileEntry &&other);
	FileEntry &operator=(FileEntry &&other);

	const std::string &getFilepath() const;
	const std::string getFullFilepath() const;

	bool isDirectory() const;
	bool isFile() const;

private:
	friend class FileList;
	std::string _filepath;
	std::string _rootDirectory;
	bool _isDir = false;
};

TS_END_PACKAGE1()

