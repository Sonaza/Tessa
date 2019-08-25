#include "Precompiled.h"
#include "ts/tessa/system/ConfigReader.h"

#include "ext/INIReader.h"
#include "ts/tessa/math/Hash.h"
#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/file/OutputFile.h"
#include <sstream>

TS_PACKAGE1(system)

namespace
{

template<class T>
T convert(const std::string &from)
{
	std::stringstream ss;
	ss << from;
	T out;
	ss >> out;
	return out;
}

}

const std::set<std::string> ConfigReader::emptySet;

ConfigReader::ConfigReader()
{
}

ConfigReader::ConfigReader(const std::string &configFilePath)
{
	open(configFilePath);
}

ConfigReader::~ConfigReader()
{

}

bool ConfigReader::open(const std::string &configFilePath)
{
	if (configFilePath.empty())
		return false;

	if (!file::utils::exists(configFilePath))
		return false;

	filepath = configFilePath;

	if (ini_parse(filepath.c_str(), &ConfigReader::parserHandler, this) < 0)
	{
		TS_LOG_ERROR("Parsing config file failed. File: %s", filepath);
		return false;
	}

	return true;
}

bool ConfigReader::save()
{
	if (filepath.empty())
	{
		TS_LOG_ERROR("Cannot save config because filepath is not set (config has not been not opened)");
		return false;
	}

	file::OutputFile output(filepath, file::OutputFileMode_WriteBinaryTruncate);

	for (const std::string &section : sections)
	{
		output.writeString(TS_FMT("[%s]\n", section));

		const Uint32 sectionHash = makeHash(section);
		for (const std::string &key : sectionKeys[sectionHash])
		{
			std::string section_dot_key = section + "." + key;
			const Uint32 hash = makeHash(section_dot_key);
			output.writeString(TS_FMT("%s = %s\n", key, values[hash]));
		}

		output.writeString("\n");
	}

	output.close();

	return true;
}

bool ConfigReader::save(const std::string &configFilePath)
{
	filepath = configFilePath;
	return save();
}

void ConfigReader::clear()
{
	sections.clear();
	sectionKeys.clear();
	values.clear();

	filepath.clear();
}

const std::set<std::string> &ConfigReader::getSections() const
{
	return sections;
}

const std::set<std::string> &ConfigReader::getKeys(const std::string &section)
{
	auto it = sectionKeys.find(makeHash(section));
	if (it != sectionKeys.end())
		return it->second;
	return ConfigReader::emptySet;
}

const std::string &ConfigReader::getString(const std::string &section_dot_key, const std::string &defaultValue) const
{
	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return it->second;
}

void ConfigReader::setString(const std::string &section_dot_key, const std::string &value)
{
	setValue(section_dot_key, value);
}

Int32 ConfigReader::getInt32(const std::string &section_dot_key, const Int32 defaultValue) const
{
	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return convert<Int32>(it->second);
}

void ConfigReader::setInt32(const std::string &section_dot_key, const Int32 value)
{
	setValue(section_dot_key, TS_FMT("%d", value));
}

Uint32 ConfigReader::getUint32(const std::string &section_dot_key, const Uint32 defaultValue) const
{
	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return convert<Uint32>(it->second);
}

void ConfigReader::setUint32(const std::string &section_dot_key, const Uint32 value)
{
	setValue(section_dot_key, TS_FMT("%u", value));
}

float ConfigReader::getFloat(const std::string &section_dot_key, const float defaultValue) const
{
	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return convert<float>(it->second);
}

void ConfigReader::setFloat(const std::string &section_dot_key, const float value, Int32 precision)
{
	setValue(section_dot_key, TS_FMT("%.*f", math::max(0, precision), value));
}

bool ConfigReader::getBoolean(const std::string &section_dot_key, const bool defaultValue) const
{
	std::string value = getString(section_dot_key, "");
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);

	if (value == "1" || value == "true" || value == "yes" || value == "on")
		return true;
	else if (value == "0" || value == "false" || value == "no" || value == "off")
		return false;

	return defaultValue;
}

void ConfigReader::setBoolean(const std::string &section_dot_key, const bool value)
{
	setValue(section_dot_key, value ? "true" : "false");
}

int ConfigReader::parserHandler(void *userdata, const char *sectionParam, const char *keyParam, const char *valueParam)
{
	ConfigReader &cfg = *(ConfigReader*)userdata;

	std::string section(sectionParam);
	std::string key(keyParam);
	std::string value(valueParam);

	cfg.sections.emplace(section);

	const Uint32 sectionHash = makeHash(section);
	cfg.sectionKeys[sectionHash].emplace(key);

	std::string section_dot_key = section + "." + key;
	const Uint32 hash = makeHash(section_dot_key);
	cfg.values[hash] = value;

	return 1;
}

Uint32 ConfigReader::makeHash(const std::string &tokenParam)
{
	std::string token = tokenParam;
	std::transform(token.begin(), token.end(), token.begin(), ::tolower);
	return math::simpleHash32(token);
}

void ConfigReader::setValue(const std::string &section_dot_key, const std::string &value)
{
	std::string section;
	std::string key;
	if (!parseSectionAndKey(section_dot_key, section, key))
		return;

	sections.emplace(section);

	const Uint32 sectionHash = makeHash(section);
	sectionKeys[sectionHash].emplace(key);

	const Uint32 hash = makeHash(section_dot_key);
	values[hash] = value;
}

bool ConfigReader::parseSectionAndKey(const std::string &section_dot_key, std::string &outSection, std::string &outKey)
{
	size_t pos = section_dot_key.find_first_of('.', 0);
	if (pos == std::string::npos)
		return false;

	outSection = section_dot_key.substr(0, pos);
	outKey = section_dot_key.substr(pos + 1);
	return true;
}

TS_END_PACKAGE1()
