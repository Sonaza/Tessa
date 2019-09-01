#include "Precompiled.h"
#include "ts/tessa/system/Commando.h"

#include "ts/tessa/math/Hash.h"

TS_PACKAGE1(system)

Commando::Commando()
{
}

Commando::Commando(int32 argc, const char **argv)
{
	parse(argc, argv);
}

Commando::Commando(int32 argc, const wchar_t **argv)
{
	parse(argc, argv);
}

void Commando::parse(int32 argc, const char **argv)
{
	flags.clear();
	parameters.clear();

	rawArgumentsList.clear();
	rawArgumentsList.insert(rawArgumentsList.begin(), argv, argv + argc);

	// First param should always be the executable path
	// though it may not be the actual executable.
	if (argc >= 1)
		executablePath = String(argv[0]);

	if (argc <= 1)
		return;

	String currentFlag;
	for (int32 i = 1; i < argc; ++i)
	{
		// Flags are considered to be tokens that start with a dash
		if (argv[i][0] == '-')
		{
			String flag(argv[i] + 1, std::strlen(argv[i]) - 1);
			String parameter;
			if ((i + 1) < argc && argv[i + 1][0] != '-')
			{
				parameter = String(argv[i + 1]);
				++i;
			}
			
			uint32 flagHash = math::simpleHash32(flag);
			flags.emplace(flagHash, std::move(parameter));
		}
		else
		{
			parameters.push_back(
				String(argv[i])
			);
		}
	}
}

void Commando::parse(int32 argc, const wchar_t **argv)
{
	flags.clear();
	parameters.clear();

	rawArgumentsList.clear();
	rawArgumentsList.insert(rawArgumentsList.begin(), argv, argv + argc);

	// First param should always be the executable path
	// though it may not be the actual executable.
	if (argc >= 1)
		executablePath = String(argv[0]);

	if (argc <= 1)
		return;

	String currentFlag;
	for (int32 i = 1; i < argc; ++i)
	{
		// Flags are considered to be tokens that start with a dash
		if (argv[i][0] == '-')
		{
			String flag(argv[i] + 1, std::wcslen(argv[i]) - 1);
			String parameter;
			if ((i + 1) < argc && argv[i + 1][0] != '-')
			{
				parameter = String(argv[i + 1]);
				++i;
			}

			uint32 flagHash = math::simpleHash32(flag);
			flags.emplace(flagHash, std::move(parameter));
		}
		else
		{
			parameters.push_back(
				String(argv[i])
			);
		}
	}
}

bool Commando::hasFlag(const String &flag) const
{
	uint32 flagHash = math::simpleHash32(flag);
	return flags.find(flagHash) != flags.end();
}

bool Commando::hasFlagParameter(const String &flag) const
{
	uint32 flagHash = math::simpleHash32(flag);
	FlagsList::const_iterator iter = flags.find(flagHash);
	if (iter == flags.end())
		return 0;
	return !iter->second.isEmpty();
}

bool Commando::getFlagParameter(const String &flag, String &outParam) const
{
	uint32 flagHash = math::simpleHash32(flag);
	FlagsList::const_iterator iter = flags.find(flagHash);
	if (iter == flags.end() && !iter->second.isEmpty())
		return false;

	outParam = iter->second;
	return true;
}

SizeType Commando::getNumParameters() const
{
	return (SizeType)parameters.size();
}

bool Commando::getNthParameter(SizeType index, String &outParam) const
{
	if (index >= parameters.size())
		return false;

	outParam = parameters[index];
	return true;
}

const String &Commando::getExecutablePath() const
{
	return executablePath;
}

TS_END_PACKAGE1()
