#include "Precompiled.h"
#include "WindowsUtils.h"

#include "ts/tessa/common/IncludeWindows.h"

TS_PACKAGE1(windows)

extern String getLastErrorAsString()
{
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return String();

	LPWSTR messageBuffer = nullptr;
	size_t size = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, nullptr);

	String message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}

TS_END_PACKAGE1()
