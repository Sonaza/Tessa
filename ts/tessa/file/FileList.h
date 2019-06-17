#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <stack>

TS_PACKAGE1(file)

class FileEntry
{
public:
	FileEntry() = default;
	FileEntry(FileEntry &&other);
	FileEntry &operator=(FileEntry &&other);

	const std::string &getFilepath() const;
	bool isDirectory() const;

private:
	friend class FileList;
	std::string _filepath;
	bool _isDir = false;
};

#define TS_FILELIST_STYLE_FILES			0b001
#define TS_FILELIST_STYLE_DIRECTORIES	0b010
#define TS_FILELIST_STYLE_RECURSIVE     0b100
#define TS_FILELIST_STYLE_ALL           TS_FILELIST_STYLE_FILES | TS_FILELIST_STYLE_DIRECTORIES

enum FileListStyle : Uint8
{
	FileListStyle_All                   = TS_FILELIST_STYLE_ALL,
	FileListStyle_All_Recursive         = TS_FILELIST_STYLE_ALL | TS_FILELIST_STYLE_RECURSIVE,
	FileListStyle_Directories           = TS_FILELIST_STYLE_DIRECTORIES,
	FileListStyle_Directories_Recursive = TS_FILELIST_STYLE_DIRECTORIES | TS_FILELIST_STYLE_RECURSIVE,
	FileListStyle_Files                 = TS_FILELIST_STYLE_FILES,
	FileListStyle_Files_Recursive       = TS_FILELIST_STYLE_FILES | TS_FILELIST_STYLE_RECURSIVE,
};

class FileList : public lang::Noncopyable
{
public:
	FileList();
	FileList(const std::string &directoryPath, bool skipDotEntries = true, FileListStyle style = FileListStyle_All);
	~FileList();

	bool open(const std::string &directoryPath, bool skipDotEntries = true, FileListStyle style = FileListStyle_All);
	void close();

	bool next(FileEntry &entry);
	bool done() const;
	void rewind();

	std::vector<FileEntry> getFullListing();

private:
	mutable std::mutex mutex;

	// Storing as void pointer to avoid having to include dirent.h in header
	struct DirectoryFrame
	{
		DirectoryFrame(void *ptr, const std::string &path)
			: ptr(ptr), rootPath(std::move(path)) {}

		void *ptr = nullptr;
		std::string rootPath;
	};
	std::stack<DirectoryFrame> _dirStack;

	std::string _directoryPath;
	FileListStyle _style = FileListStyle_All;
	bool _skipDotEntries = true;
	bool _done = false;
};

TS_END_PACKAGE1()

