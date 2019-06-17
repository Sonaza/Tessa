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

TS_PACKAGE2(file, utils)

extern bool hasTrailingSlash(const std::string &path);

extern void removeTrailingSlashes(std::string &path);
extern std::string removeTrailingSlashesCopy(const std::string &path);

extern std::string joinPaths(const std::string &left, const std::string &right, char delimiter = TS_SYSTEM_PATH_DELIMITER);
extern std::string normalizePath(const std::string &path, char delimiter = TS_SYSTEM_PATH_DELIMITER);

extern std::string getDirname(const std::string &path, const std::string &delimiters = TS_ALL_PATH_DELIMITERS);
extern std::string getBasename(const std::string &path, const std::string &delimiters = TS_ALL_PATH_DELIMITERS);

// Returns the substring after the last period (.) or empty if not found before the last path delimiter
// Does not check if the given path is a directory or file
extern std::string getExtension(const std::string &path);

extern bool exists(const std::string &path);
extern bool isFile(const std::string &path);
extern bool isDirectory(const std::string &path);
// extern bool exists(const std::wstring &path);

TS_END_PACKAGE2()
