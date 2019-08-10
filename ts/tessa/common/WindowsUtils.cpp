#include "Precompiled.h"
#include "WindowsUtils.h"

#include "ts/tessa/common/IncludeWindows.h"

TS_PACKAGE1(windows)

extern std::string getLastErrorAsString()
{
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string();

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);

	std::string message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}

extern std::wstring getLastErrorAsWideString()
{
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::wstring();

	LPWSTR messageBuffer = nullptr;
	size_t size = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, nullptr);

	std::wstring message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}

TS_END_PACKAGE1()
