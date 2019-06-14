#include "Precompiled.h"
#include "ts/tessa/system/ConfigReader.h"

#include "ts/tessa/math/Hash.h"
#include "INIReader.h"
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

ConfigReader::ConfigReader(const std::string &configfile)
{
	parse(configfile);
}

ConfigReader::~ConfigReader()
{

}

bool ConfigReader::isLoaded() const
{
	return loaded;
}

bool ConfigReader::parse(const std::string &file)
{
	if (file.empty())
		return false;

	sections.clear();
	sectionFields.clear();
	values.clear();

	filepath = file;

	if (ini_parse(filepath.c_str(), &ConfigReader::parserHandler, this) < 0)
	{
		TS_LOG_ERROR("Parsing config file failed. File: %s", filepath);
		return false;
	}

	loaded = true;

	return true;
}

const std::set<std::string> &ConfigReader::getSections() const
{
	return sections;
}

const std::set<std::string> &ConfigReader::getFields(const std::string &section)
{
	auto it = sectionFields.find(makeHash(section));
	if (it != sectionFields.end())
		return it->second;
	return ConfigReader::emptySet;
}

const std::string &ConfigReader::getString(const std::string &section, const std::string &key, const std::string &defaultValue) const
{
	const std::string section_dot_key = section + "." + key;
	return getString(section_dot_key, defaultValue);
}

const std::string &ConfigReader::getString(const std::string &section_dot_key, const std::string &defaultValue) const
{
	if (!loaded)
		return defaultValue;

	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return it->second;
}

Int32 ConfigReader::getInt32(const std::string &section, const std::string &key, const Int32 defaultValue) const
{
	const std::string section_dot_key = section + "." + key;
	return getInt32(section_dot_key, defaultValue);
}

Int32 ConfigReader::getInt32(const std::string &section_dot_key, const Int32 defaultValue) const
{
	if (!loaded)
		return defaultValue;

	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return convert<Int32>(it->second);
}

Uint32 ConfigReader::getUint32(const std::string &section, const std::string &key, const Uint32 defaultValue) const
{
	const std::string section_dot_key = section + "." + key;
	return getUint32(section_dot_key, defaultValue);
}

Uint32 ConfigReader::getUint32(const std::string &section_dot_key, const Uint32 defaultValue) const
{
	if (!loaded)
		return defaultValue;

	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return convert<Uint32>(it->second);
}

float ConfigReader::getFloat(const std::string &section, const std::string &key, const float defaultValue) const
{
	const std::string section_dot_key = section + "." + key;
	return getFloat(section_dot_key, defaultValue);
}

float ConfigReader::getFloat(const std::string &section_dot_key, const float defaultValue) const
{
	if (!loaded)
		return defaultValue;

	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return convert<float>(it->second);
}

bool ConfigReader::getBoolean(const std::string &section, const std::string &key, const bool defaultValue) const
{
	const std::string section_dot_key = section + "." + key;
	return getBoolean(section_dot_key, defaultValue);
}

bool ConfigReader::getBoolean(const std::string &section_dot_key, const bool defaultValue) const
{
	if (!loaded)
		return defaultValue;

	std::string value = getString(section_dot_key, "");
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);

	if (value == "1" || value == "true" || value == "yes" || value == "on")
		return true;
	else if (value == "0" || value == "false" || value == "no" || value == "off")
		return false;

	return defaultValue;
}

int ConfigReader::parserHandler(void *userdata, const char *sectionParam, const char *keyParam, const char *valueParam)
{
	ConfigReader &cfg = *(ConfigReader*)userdata;

	const std::string section(sectionParam);
	const std::string key(keyParam);
	const std::string value(valueParam);

	cfg.sections.emplace(section);

	const Uint32 sectionHash = makeHash(section);
	cfg.sectionFields[sectionHash].emplace(key);

	const std::string section_dot_key = section + "." + key;
	const Uint32 hash = makeHash(section_dot_key);
	cfg.values[hash] = value;

	return 1;
}

Uint32 ConfigReader::makeHash(const std::string &tokenParam)
{
	std::string token = tokenParam;
	std::transform(token.begin(), token.end(), token.begin(), ::tolower);
	return math::hash::simpleHash32(token);
}

TS_END_PACKAGE1()
