#pragma once

#include "ts/tessa/time/Time.h"

#if TS_PLATFORM == TS_WINDOWS
	#define TS_SYSTEM_PATH_DELIMITER '\\'
#elif TS_PLATFORM == TS_LINUX
	#define TS_SYSTEM_PATH_DELIMITER '/'
#else
	#define TS_SYSTEM_PATH_DELIMITER '/'
#endif

#define TS_ALL_PATH_DELIMITERS "/\\"

TS_PACKAGE1(file)

/* Returns true if the given path is an absolute path.
 * Windows: checks if the path has drive letter or is prefixed with two backslashes (network share).
 * Other: checks if the first character of the path is a forward slash.
 */
extern bool isAbsolutePath(const String &path);

/* Returns true if the given path has trailing slashes (checks for both forward and backslashes)
 */
extern bool hasTrailingSlash(const String &path);

/* Removes all trailing slashes, modifying the given path in place.
 */
extern void removeTrailingSlashes(String &path);

/* Returns the the two sections joined together, while making sure there aren't duplicate path delimiters.
 */
extern String joinPaths(const String &left, const String &right, string::Character delimiter = TS_SYSTEM_PATH_DELIMITER);

/* Returns a normalized path by collapsing all relational segments to their simplest form.
 * Additionally all path delimiters are replaced with the one given.
 */
extern String normalizePath(const String &path, string::Character delimiter = TS_SYSTEM_PATH_DELIMITER);

/* Returns if given absolute path is subpath of another absolute path.
 */
extern bool pathIsSubpath(const String &rootPath, const String &otherPath);

/* Returns the directory path without file name. First checks if the path is a directory,
 * if it isn't then it simply removes everything past the last path delimiter.
 */
extern String getDirname(const String &path, const String &delimiters = TS_ALL_PATH_DELIMITERS);

/* Returns file's name with directory path stripped.
 * Optionally file extension can also be removed (removes everything past the last period (.))
 */
extern String getBasename(const String &path, bool stripExtension = false, const String &delimiters = TS_ALL_PATH_DELIMITERS);

/* Returns the substring after the last period (.)
 * or empty if not found before the last path delimiter (does not confirm if path was a file or a directory).
 */
extern String getExtension(const String &path);

/* Returns true if given file path exists.
 */
extern bool exists(const String &path);

/* Returns true if the given path is a file.
 */
extern bool isFile(const String &path);

/* Returns true if the given path is a directory.
*/
extern bool isDirectory(const String &path);

/* Attempts to remove the given file and returns true if the operation was successful.
 */
extern bool removeFile(const String &path);

extern bool getFileModifiedTime(const String &path, int64 &modifiedTime);

/* Returns the base directory path where the current executable is located.
 */
extern String getExecutableDirectory();

/* Returns the current working directory
 */
extern String getWorkingDirectory();

/* Changes the current working directory to the given path.
 */
extern void setWorkingDirectory(const String &path);

TS_END_PACKAGE1()
