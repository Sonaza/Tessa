#pragma once

#include <string>

#if TS_PLATFORM == TS_WINDOWS
	#define TS_SYSTEM_PATH_DELIMITER '\\'
#elif TS_PLATFORM == TS_LINUX
	#define TS_SYSTEM_PATH_DELIMITER '/'
#else
	#define TS_SYSTEM_PATH_DELIMITER '/'
#endif

#define TS_ALL_PATH_DELIMITERS "/\\"

#define TS_SYSTEM_PATH_DELIMITER_WIDE L'\\'
#define TS_ALL_PATH_DELIMITERS_WIDE L"/\\"

TS_PACKAGE2(file, utils)

extern bool isAbsolutePath(const std::string &path);
extern bool isAbsolutePath(const std::wstring &path);

extern bool hasTrailingSlash(const std::string &path);
extern bool hasTrailingSlash(const std::wstring &path);

extern void removeTrailingSlashes(std::string &path);
extern std::string removeTrailingSlashesCopy(const std::string &path);

extern void removeTrailingSlashes(std::wstring &path);
extern std::wstring removeTrailingSlashesCopy(const std::wstring &path);

extern std::string joinPaths(const std::string &left, const std::string &right, char delimiter = TS_SYSTEM_PATH_DELIMITER);
extern std::wstring joinPaths(const std::wstring &left, const std::wstring &right, wchar_t delimiter = TS_SYSTEM_PATH_DELIMITER_WIDE);

extern std::string normalizePath(const std::string &path, char delimiter = TS_SYSTEM_PATH_DELIMITER);
extern std::wstring normalizePath(const std::wstring &path, wchar_t delimiter = TS_SYSTEM_PATH_DELIMITER_WIDE);

extern std::string getDirname(const std::string &path, const std::string &delimiters = TS_ALL_PATH_DELIMITERS);
extern std::wstring getDirname(const std::wstring &path, const std::wstring &delimiters = TS_ALL_PATH_DELIMITERS_WIDE);

extern std::string getBasename(const std::string &path, bool stripExtension = false, const std::string &delimiters = TS_ALL_PATH_DELIMITERS);
extern std::wstring getBasename(const std::wstring &path, bool stripExtension = false, const std::wstring &delimiters = TS_ALL_PATH_DELIMITERS_WIDE);

// Returns the substring after the last period (.) or empty if not found before the last path delimiter
// Does not check if the given path is a directory or file
extern std::string getExtension(const std::string &path);
extern std::wstring getExtension(const std::wstring &path);

extern bool exists(const std::string &path);
extern bool exists(const std::wstring &path);

extern bool isFile(const std::string &path);
extern bool isFile(const std::wstring &path);

extern bool isDirectory(const std::string &path);
extern bool isDirectory(const std::wstring &path);

extern bool removeFile(const std::string &path);
extern bool removeFile(const std::wstring &path);

extern std::string getExecutableDirectory();
extern std::wstring getExecutableDirectoryWide();

extern std::string getWorkingDirectory();
extern std::wstring getWorkingDirectoryWide();

extern void setWorkingDirectory(const std::string &path);
extern void setWorkingDirectory(const std::wstring &path);

TS_END_PACKAGE2()
