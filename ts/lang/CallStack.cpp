#include "Precompiled.h"
#include "CallStack.h"

#include "ts/tessa/lang/StringUtils.h"

#if TS_PLATFORM == TS_WINDOWS
	#include "StackWalker.h"
#endif

TS_PACKAGE1(lang)

#if TS_PLATFORM == TS_WINDOWS

class StackWalkerToLog : public StackWalker
{
	SizeType numBacktrack = 0;
public:
	StackWalkerToLog(SizeType numBacktrack = 0)
		: StackWalker(RetrieveLine)
		, numBacktrack(numBacktrack)
	{
	}

	virtual void OnOutput(LPCSTR text, OutputType type) override
	{
		if (type == OutputCallstackEntry || type == OutputDbgHelpError)
		{
			if (numBacktrack > 0)
			{
				numBacktrack--;
				return;
			}
			TS_PRINTF("%s", text);
		}
	}
};

class StackWalkerToStringVector : public StackWalker
{
	SizeType numBacktrack = 0;
public:
	StackWalkerToStringVector(SizeType numBacktrack = 0)
		: StackWalker(RetrieveLine)
		, numBacktrack(numBacktrack)
	{
	}

	virtual void OnOutput(LPCSTR text, OutputType type) override
	{
		if (type == OutputCallstackEntry || type == OutputDbgHelpError)
		{
			if (numBacktrack > 0)
			{
				numBacktrack--;
				return;
			}
			stack.push_back(std::string(text));
		}
	}

	std::vector<std::string> stack;
};

#endif

void CallStack::printCallstack()
{
#if TS_PLATFORM == TS_WINDOWS
	StackWalkerToLog sw(2);
	sw.ShowCallstack();
#else
	TS_ASSERT(!"Not implemented on this platform.");
#endif
}

std::vector<std::string> CallStack::getCallstack()
{
#if TS_PLATFORM == TS_WINDOWS
	StackWalkerToStringVector sw(2);
	sw.ShowCallstack(GetCurrentThread());

	return sw.stack;
#else
	TS_ASSERT(!"Not implemented on this platform.");
	return std::vector<std::string>();
#endif
}

std::string CallStack::getCallstackAsString()
{
#if TS_PLATFORM == TS_WINDOWS
	std::vector<std::string> stack = getCallstack();
	return lang::utils::joinString(stack, "");
#else
	TS_ASSERT(!"Not implemented on this platform.");
	return std::string();
#endif
}

TS_END_PACKAGE1()
