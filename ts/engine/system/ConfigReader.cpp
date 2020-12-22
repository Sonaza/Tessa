#include "Precompiled.h"
#include "ts/engine/system/ConfigReader.h"

#include "ext/INIReader.h"
#include "ts/math/Hash.h"
#include "ts/file/FileUtils.h"
#include "ts/file/InputFile.h"
#include "ts/file/OutputFile.h"

#include "ts/string/StringUtils.h"

#include <sstream>
#include <cstdio>

TS_PACKAGE2(engine, system)

namespace
{

template<class T>
T convert(const String &from)
{
	std::stringstream ss;
	ss << from.toAnsiString(); // For converting numeric values, they should always be ansi
	T out;
	ss >> out;
	return out;
}

}

const std::set<String> ConfigReader::emptySet;

ConfigReader::ConfigReader()
{
}

ConfigReader::ConfigReader(const String &configFilePath)
{
	open(configFilePath);
}

ConfigReader::~ConfigReader()
{

}

bool ConfigReader::open(const String &configFilePath)
{
	if (configFilePath.isEmpty())
		return false;

	if (!file::exists(configFilePath))
		return false;

	file::InputFile file(configFilePath, file::InputFileMode_ReadBinary);
	if (!file.isOpen())
		return false;

	if (ini_parse_stream(&ConfigReader::customIniReader, &file, &ConfigReader::parserHandler, this) < 0)
	{
		TS_LOG_ERROR("Parsing config file failed. File: %s", filepath);
		return false;
	}

	filepath = configFilePath;

	return true;
}

char *ConfigReader::customIniReader(char *str, int count, void *stream)
{
	file::InputFile &file = *(file::InputFile*)stream;
	PosType bytesRead = file.readLine(str, count);
	if (bytesRead <= 0)
		return nullptr;
	return str;
}

bool ConfigReader::save()
{
	if (filepath.isEmpty())
	{
		TS_LOG_ERROR("Cannot save config because filepath is not set (config has not been not opened)");
		return false;
	}

	file::OutputFile output(filepath, file::OutputFileMode_WriteBinaryTruncate);

	for (const String &section : sections)
	{
		output.writeString(TS_FMT("[%s]\n", section));

		const uint32_t sectionHash = makeHash(section);
		for (const String &key : sectionKeys[sectionHash])
		{
			String section_dot_key = section + "." + key;
			const uint32_t hash = makeHash(section_dot_key);
			output.writeString(TS_FMT("%s = %s\n", key, values[hash]));
		}

		output.writeString("\n");
	}

	output.close();

	return true;
}

bool ConfigReader::save(const String &configFilePath)
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

const std::set<String> &ConfigReader::getSections() const
{
	return sections;
}

const std::set<String> &ConfigReader::getKeys(const String &section)
{
	auto it = sectionKeys.find(makeHash(section));
	if (it != sectionKeys.end())
		return it->second;
	return ConfigReader::emptySet;
}

const String &ConfigReader::getString(const String &section_dot_key, const String &defaultValue) const
{
	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return it->second;
}

void ConfigReader::setString(const String &section_dot_key, const String &value)
{
	setValue(section_dot_key, value);
}

int32_t ConfigReader::getInt32(const String &section_dot_key, const int32_t defaultValue) const
{
	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return convert<int32_t>(it->second);
}

void ConfigReader::setInt32(const String &section_dot_key, const int32_t value)
{
	setValue(section_dot_key, TS_FMT("%d", value));
}

uint32_t ConfigReader::getUint32(const String &section_dot_key, const uint32_t defaultValue) const
{
	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return convert<uint32_t>(it->second);
}

void ConfigReader::setUint32(const String &section_dot_key, const uint32_t value)
{
	setValue(section_dot_key, TS_FMT("%u", value));
}

float ConfigReader::getFloat(const String &section_dot_key, const float defaultValue) const
{
	auto it = values.find(makeHash(section_dot_key));
	if (it == values.end())
		return defaultValue;
	return convert<float>(it->second);
}

void ConfigReader::setFloat(const String &section_dot_key, const float value, int32_t precision)
{
	setValue(section_dot_key, TS_FMT("%.*f", math::max(0, precision), value));
}

bool ConfigReader::getBoolean(const String &section_dot_key, const bool defaultValue) const
{
	String value = getString(section_dot_key, "");
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);

	if (value == "1" || value == "true" || value == "yes" || value == "on")
		return true;
	else if (value == "0" || value == "false" || value == "no" || value == "off")
		return false;

	return defaultValue;
}

void ConfigReader::setBoolean(const String &section_dot_key, const bool value)
{
	setValue(section_dot_key, value ? "true" : "false");
}

int ConfigReader::parserHandler(void *userdata, const char *sectionParam, const char *keyParam, const char *valueParam)
{
	ConfigReader &cfg = *(ConfigReader*)userdata;

	String section(sectionParam);
	String key(keyParam);
	String value(valueParam);

	cfg.sections.emplace(section);

	const uint32_t sectionHash = makeHash(section);
	cfg.sectionKeys[sectionHash].emplace(key);

	String section_dot_key = section + "." + key;
	const uint32_t hash = makeHash(section_dot_key);
	cfg.values[hash] = value;

	return 1;
}

uint32_t ConfigReader::makeHash(const String &tokenParam)
{
	String token = tokenParam;
	std::transform(token.begin(), token.end(), token.begin(), ::tolower);
	return math::simpleHash32(token);
}

void ConfigReader::setValue(const String &section_dot_key, const String &value)
{
	String section;
	String key;
	if (!parseSectionAndKey(section_dot_key, section, key))
		return;

	sections.emplace(section);

	const uint32_t sectionHash = makeHash(section);
	sectionKeys[sectionHash].emplace(key);

	const uint32_t hash = makeHash(section_dot_key);
	values[hash] = value;
}

bool ConfigReader::parseSectionAndKey(const String &section_dot_key, String &outSection, String &outKey)
{
	std::vector<String> parts = string::splitString(section_dot_key, ".");
	if (parts.size() >= 2)
	{
		outSection = parts[0];
		outKey = parts[1];
		return true;
	}
	return false;
}

TS_END_PACKAGE2()
