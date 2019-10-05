#pragma once

TS_PACKAGE1(windows)

// returns the last Win32 error, in string format. Returns an empty string if there is no error.
extern String getLastErrorAsString();

extern bool openExplorerToFile(const String &filepath);

extern bool openFileWithDialog(const String &filepath);

extern BigSizeType convertLargeIntegerTo64bit(SizeType lowPart, SizeType highPart);

extern int32 getWindowsVersion();

TS_END_PACKAGE1()
