#include "Precompiled.h"
#include "ts/tessa/system/Commando.h"

#include "ts/tessa/math/Hash.h"
#include <algorithm>

TS_PACKAGE1(system)

Commando::Commando()
{
}

Commando::~Commando()
{
}

bool Commando::parse(const wchar_t *args)
{
	SizeType argsLength = (SizeType)std::wcslen(args);
	if (argsLength == 0)
		return true;

	std::vector<String> compiledArgs;

	bool quotes = false;

	const wchar_t *startPtr = args;
	const wchar_t *ptr = startPtr;

	const wchar_t *currentStart = nullptr;
	SizeType currentLength = 0;

	while (*ptr != '\0')
	{
		bool appendString = false;

		if (*ptr == L'"')
		{
			if (quotes)
			{
				const wchar_t *nextPtr = ptr + 1;
				if (*nextPtr == '\0')
					break;

				if (*nextPtr == ' ')
				{
					appendString = true;
					quotes = false;

					++ptr;
				}
				else
				{
					currentLength++;
				}
			}
			else
			{
				quotes = true;
			}
		}
		else if (*ptr == ' ' && quotes == false)
		{
			appendString = true;
		}
		else
		{
			if (currentStart == nullptr)
				currentStart = ptr;
			currentLength++;
		}

		if (appendString)
		{
			if (currentStart != nullptr && currentLength > 0)
			{
				String str(currentStart, currentLength);
				compiledArgs.push_back(str);
			}

			currentStart = nullptr;
			currentLength = 0;
		}

		++ptr;
	}

	if (currentStart != nullptr && currentLength > 0)
	{
		String str(currentStart, currentLength);
		compiledArgs.push_back(str);
	}

	return parse(compiledArgs);
}

bool Commando::parse(int32 argc, const char **argv)
{
	std::vector<String> args;
	args.reserve(argc);
	for (int32 index = 0; index < argc; ++index)
	{
		args.push_back(String(argv[index]));
	}
	return parse(args);
}

bool Commando::parse(int32 argc, const wchar_t **argv)
{
	std::vector<String> args;
	args.reserve(argc);
	for (int32 index = 0; index < argc; ++index)
	{
		args.push_back(String(argv[index]));
	}
	return parse(args);
}

bool Commando::parse(const std::vector<String> &args)
{
	m_flags.clear();
	m_parameters.clear();

	if (args.empty())
		return true;

	for (uint32 index = 0; index < args.size(); ++index)
	{
		String arg = args[index];

		// Flags are considered to be tokens that start with a dash
		if (args[index][0] == '-')
		{
			// Erase all dashes from the flag
			String::iterator dashIter = std::find_if(arg.begin(), arg.end(), [](char32_t c)
			{
				return c != '-';
			});
			arg.erase(arg.begin(), dashIter);

			String parameter;

			// Only single-dash flags can have parameters
			PosType numDashes = std::distance(arg.begin(), dashIter);
			if (numDashes == 1 && (index + 1) < args.size() && args[index + 1][0] != '-')
			{
				parameter = String(args[index + 1]);
				++index;
			}
			
			uint32 flagHash = math::simpleHash32(arg);
			m_flags.emplace(flagHash, std::move(parameter));
		}
		else
		{
			m_parameters.push_back(std::move(arg));
		}
	}

	return true;
}

bool Commando::hasFlag(const String &flag) const
{
	uint32 flagHash = math::simpleHash32(flag);
	return m_flags.find(flagHash) != m_flags.end();
}

bool Commando::hasFlagParameter(const String &flag) const
{
	uint32 flagHash = math::simpleHash32(flag);
	FlagsList::const_iterator iter = m_flags.find(flagHash);
	if (iter == m_flags.end())
		return 0;
	return !iter->second.isEmpty();
}

bool Commando::getFlagParameter(const String &flag, String &outParam) const
{
	uint32 flagHash = math::simpleHash32(flag);
	FlagsList::const_iterator iter = m_flags.find(flagHash);
	if (iter == m_flags.end() && !iter->second.isEmpty())
		return false;

	outParam = iter->second;
	return true;
}

SizeType Commando::getNumParameters() const
{
	return (SizeType)m_parameters.size();
}

bool Commando::getNthParameter(SizeType index, String &outParam) const
{
	if (index >= m_parameters.size())
		return false;

	outParam = m_parameters[index];
	return true;
}

TS_END_PACKAGE1()
