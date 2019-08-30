#include "Precompiled.h"
#include "ts/tessa/system/Commando.h"

#include "ts/tessa/math/Hash.h"
#include <locale>
#include <codecvt>

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

	// First param should always be the application executable path
	if (argc >= 1)
	{
		executablePath_utf8 = std::string(argv[0]);
		executablePath_utf16 = convertU8toU16(executablePath_utf8);
	}

	if (argc <= 1)
		return;

	std::string currentFlag;
	for (int32 i = 1; i < argc; ++i)
	{
		// Flags are considered to be tokens that start with a dash
		if (argv[i][0] == '-')
		{
			std::string flag(argv[i] + 1, strlen(argv[i]) - 1);
			std::string parameter;
			if ((i + 1) < argc && argv[i + 1][0] != '-')
			{
				parameter = std::string(argv[i + 1]);
				++i;
			}
			
			uint32 flagHash = math::simpleHash32(flag);
			flags.emplace(flagHash, std::move(parameter));
		}
		else
		{
			parameters.push_back(
				std::string(argv[i])
			);
		}
	}
}

void Commando::parse(int32 argc, const wchar_t **argv)
{
	flags.clear();
	parameters.clear();

	rawArgumentsListW.clear();
	rawArgumentsListW.insert(rawArgumentsListW.begin(), argv, argv + argc);

	// First param should always be the application executable path
	if (argc >= 1)
	{
		executablePath_utf16 = std::wstring(argv[0]);
		executablePath_utf8 = convertU16toU8(executablePath_utf16);
	}

	if (argc <= 1)
		return;

	std::wstring currentFlag;
	for (int32 i = 1; i < argc; ++i)
	{
		// Flags are considered to be tokens that start with a dash
		if (argv[i][0] == '-')
		{
			std::wstring flag(argv[i] + 1, wcslen(argv[i]) - 1);
			std::wstring parameter;
			if ((i + 1) < argc && argv[i + 1][0] != '-')
			{
				parameter = std::wstring(argv[i + 1]);
				++i;
			}

			// Always make flag name hashes with utf8
			uint32 flagHash = math::simpleHash32(convertU16toU8(flag));
			flags.emplace(flagHash, convertU16toU8(parameter));
		}
		else
		{
			parameters.push_back(convertU16toU8(argv[i]));
		}
	}
}

bool Commando::hasFlag(const std::string &flag) const
{
	uint32 flagHash = math::simpleHash32(flag);
	return flags.find(flagHash) != flags.end();
}

bool Commando::hasFlag(const std::wstring &flag) const
{
	uint32 flagHash = math::simpleHash32(convertU16toU8(flag));
	return flags.find(flagHash) != flags.end();
}

bool Commando::hasFlagParameter(const std::string &flag) const
{
	uint32 flagHash = math::simpleHash32(flag);
	FlagsList::const_iterator iter = flags.find(flagHash);
	if (iter == flags.end())
		return 0;
	return !iter->second.empty();
}

bool Commando::hasFlagParameter(const std::wstring &flag) const
{
	uint32 flagHash = math::simpleHash32(convertU16toU8(flag));
	FlagsList::const_iterator iter = flags.find(flagHash);
	if (iter == flags.end())
		return 0;
	return !iter->second.empty();
}

bool Commando::getFlagParameter(const std::string &flag, std::string &outParam) const
{
	uint32 flagHash = math::simpleHash32(flag);
	FlagsList::const_iterator iter = flags.find(flagHash);
	if (iter == flags.end() && !iter->second.empty())
		return false;

	outParam = iter->second;
	return true;
}

bool Commando::getFlagParameter(const std::wstring &flag, std::wstring &outParam) const
{
	uint32 flagHash = math::simpleHash32(convertU16toU8(flag));
	FlagsList::const_iterator iter = flags.find(flagHash);
	if (iter == flags.end() && !iter->second.empty())
		return false;

	outParam = convertU8toU16(iter->second);
	return true;
}

SizeType Commando::getNumParameters() const
{
	return (SizeType)parameters.size();
}

bool Commando::getNthParameter(SizeType index, std::string &outParam) const
{
	if (index >= parameters.size())
		return false;

	outParam = parameters[index];
	return true;
}

bool Commando::getNthParameter(SizeType index, std::wstring &outParam) const
{
	if (index >= parameters.size())
		return false;

	outParam = convertU8toU16(parameters[index]);
	return true;
}

const std::string &Commando::getExecutablePath() const
{
	return executablePath_utf8;
}

const std::wstring &Commando::getExecutablePathWide() const
{
	return executablePath_utf16;
}

std::wstring Commando::convertU8toU16(const std::string &str) const
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str.c_str());
}

std::string Commando::convertU16toU8(const std::wstring &str) const
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(str.c_str());
}

TS_END_PACKAGE1()
