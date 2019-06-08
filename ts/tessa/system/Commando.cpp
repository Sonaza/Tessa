#include "Precompiled.h"
#include "ts/tessa/system/Commando.h"

TS_PACKAGE1(system)

Commando::Commando()
{
}

Commando::Commando(Int32 argc, const char **argv)
{
	parse(argc, argv);
}

void Commando::parse(Int32 argc, const char **argv)
{
	flags.clear();
	parameters.clear();

	// First param should always be the application executable path
	if (argc >= 1)
		executablePath = std::string(argv[0]);

	if (argc <= 1)
		return;

	std::string currentFlag;
	for (Int32 i = 1; i < argc; ++i)
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
			
			flags.emplace(std::move(flag), std::move(parameter));
		}
		else
		{
			parameters.push_back(std::string(argv[i]));
		}
	}
}

bool Commando::hasFlag(const std::string &flag) const
{
	return flags.find(flag) != flags.end();
}

bool Commando::hasFlagParameter(const std::string &flag) const
{
	FlagsList::const_iterator iter = flags.find(flag);
	if (iter == flags.end())
		return 0;
	return !iter->second.empty();
}

bool Commando::getFlagParameter(const std::string &flag, std::string &outParam) const
{
	FlagsList::const_iterator iter = flags.find(flag);
	if (iter == flags.end() && !iter->second.empty())
		return false;

	outParam = iter->second;
	return true;
}

SizeType Commando::getNumParameters() const
{
	return (SizeType)parameters.size();
}

bool Commando::getNthParameter(SizeType index, std::string &outParam)
{
	if (index >= parameters.size())
		return false;

	outParam = parameters[index];
	return true;
}

const std::string &Commando::getExecutablePath() const
{
	return executablePath;
}

TS_END_PACKAGE1()

