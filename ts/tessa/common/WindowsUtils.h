#pragma once

TS_PACKAGE1(windows)

// returns the last Win32 error, in string format. Returns an empty string if there is no error.
extern String getLastErrorAsString();

TS_END_PACKAGE1()
