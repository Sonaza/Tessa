#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS
	#include "ts/tessa/common/IncludeWindows.h"
	#include <codecvt>
	#include <sstream>
#endif

#include "ts/tessa/lang/StringUtils.h"

TS_PACKAGE0()

namespace
{

enum DialogAction
{
	Debug,
	Ignore,
	Abort,
};

DialogAction dialog(const char *expression, const char *message, const char *filepath, const unsigned int line)
{
#if TS_PLATFORM == TS_WINDOWS
	std::wstringstream msg;

	msg << "Assertion failure!\n\n";
	
	if (message)
		msg << message << "\n\n";

	msg << "Expression: " << expression << "\n"
		<< "File: " << filepath << "\n"
		<< "Line: " << line << "\n";

	Int32 button = MessageBoxW(nullptr, msg.str().c_str(), L"Assertion failure", MB_ABORTRETRYIGNORE | MB_ICONERROR);
	switch (button)
	{
		case IDABORT: return Abort;
		case IDRETRY: return Debug;
		case IDIGNORE: return Ignore;
	}
#endif
	return Debug;
}

}

extern bool _assert_impl(const char *expression, const char *message, const char *filepath, const unsigned int line)
{
	if (message != nullptr)
	{
		std::string messageNoLinebreaks = message;
		lang::utils::replaceCharacter(messageNoLinebreaks, '\n', ' ');
		TS_PRINTF("Assertion failed (%s): %s in %s on line %u\n", messageNoLinebreaks, expression, filepath, line);
	}
	else
	{
		TS_PRINTF("Assertion failed: %s in %s on line %u\n", expression, filepath, line);
	}

	DialogAction action = dialog(expression, message, filepath, line);
	switch (action)
	{
	case Debug:  return true;
	case Ignore: return false;
	case Abort:  exit(1337);
	}
	return false;
}

TS_END_PACKAGE0()
