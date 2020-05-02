#pragma once

TS_PACKAGE1(linux)

extern bool openExplorerToFile(const String &filepath);

extern bool openFileWithDialog(const String &filepath);

extern bool isDebuggerAttached();

TS_END_PACKAGE1()
