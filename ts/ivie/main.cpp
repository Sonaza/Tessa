#include "Precompiled.h"
#include "Application.h"

#include <locale>
#include <codecvt>
#include <fstream>

#include "ts/tessa/threading/Thread.h"
#include "ts/tessa/common/Log.h"
#include "ts/tessa/file/FileUtils.h"

// #if TS_PLATFORM == TS_WINDOWS
#include "ts/tessa/common/IncludeWindows.h"

int wmain(int argc, const wchar_t **argv)
{
	ts::threading::Thread::setMainThread(ts::threading::Thread::getCurrentThread());
	
// 	ts::file::utils::setWorkingDirectory(ts::file::utils::getExecutableDirectoryWide());

// 	MessageBoxW(nullptr, argv[1], L"Potato", MB_OK);

	/*TS_PRINTF("=== UTF16 ===\n");
	
	std::wstring utf16str(argv[1]);

	const char *wstr = reinterpret_cast<const char*>(argv[1]);
	size_t len = wcslen(argv[1]) * sizeof(wchar_t);
	for (size_t i = 0; i < len; ++i)
	{
		TS_PRINTF("%02X ", wstr[i]);
	}
	TS_PRINTF("\n");
	TS_PRINTF("30 A4 30 E1 30 B8 00 2E 00 6A 00 70 00 67\n");

	TS_PRINTF("\n\n");

	TS_PRINTF("=== UTF8 ===\n");

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	std::string utf8str = converter.to_bytes(argv[1]);
	for (size_t i = 0; i < utf8str.size(); ++i)
	{
		TS_PRINTF("%02X ", (utf8str[i] & 0xFF));
	}

	TS_PRINTF("\n");
	TS_PRINTF("E3 82 A4 E3 83 A1 E3 82 B8 2E 6A 70 67\n");

	std::ifstream file;
	file.open(utf16str, std::ios_base::in | std::ios_base::binary);

	if (file.good())
	{
		char buffer[10] = { 0 };
		file.read(buffer, 10);

		for (size_t i = 0; i < 10; ++i)
		{
			TS_PRINTF("%02X ", (buffer[i] & 0xFF));
		}
		TS_PRINTF("\n");
	}
	else
	{
		TS_PRINTF("File open failed!\n");
	}

	return 0;*/

	ts::app::Application app(argc, argv);
	int returnCode = app.launch();
	return returnCode;
}