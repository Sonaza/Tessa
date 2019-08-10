#pragma once

#include <string>

TS_PACKAGE1(file)

class FileEntryW
{
public:
	FileEntryW() = default;
	FileEntryW(FileEntryW &&other);
	FileEntryW &operator=(FileEntryW &&other);

	const std::wstring &getFilepath() const;
	const std::wstring getFullFilepath() const;

	bool isDirectory() const;
	bool isFile() const;

private:
	friend class FileListW;
	std::wstring _filepath;
	std::wstring _rootDirectory;
	bool _isDir = false;
};

TS_END_PACKAGE1()

