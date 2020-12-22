#pragma once

#include <string>
#include <set>
#include <unordered_map>

TS_PACKAGE2(engine, system)

class ConfigReader : public lang::Noncopyable
{
public:
	ConfigReader();
	explicit ConfigReader(const String &configFilePath);
	~ConfigReader();

	// Opens specified config file and parses it.
	bool open(const String &configFilePath);

	// Saves current configuration to the previously opened config file or specified file.
	bool save();
	bool save(const String &configFilePath);

	void clear();

	const std::set<String> &getSections() const;
	const std::set<String> &getKeys(const String &section);

	const String &getString(const String &section_dot_key, const String &defaultValue) const;
	void setString(const String &section_dot_key, const String &value);

	int32_t getInt32(const String &section_dot_key, const int32_t defaultValue) const;
	void setInt32(const String &section_dot_key, const int32_t value);

	uint32_t getUint32(const String &section_dot_key, const uint32_t defaultValue) const;
	void setUint32(const String &section_dot_key, const uint32_t value);

	float getFloat(const String &section_dot_key, const float defaultValue) const;
	void setFloat(const String &section_dot_key, const float value, int32_t precision = 3);

	bool getBoolean(const String &section_dot_key, const bool defaultValue) const;
	void setBoolean(const String &section_dot_key, const bool value);

protected:
	static char *customIniReader(char *str, int count, void *stream);
	static int parserHandler(void *userdata, const char *section, const char *key, const char *value);
	static uint32_t makeHash(const String &token);

	void setValue(const String &section_dot_key, const String &value);

	bool parseSectionAndKey(const String &section_dot_key, String &outSection, String &outKey);

	std::set<String> sections;
	std::unordered_map<uint32_t, std::set<String>> sectionKeys;
	std::unordered_map<uint32_t, String> values;

	static const std::set<String> emptySet;

	String filepath;
};


TS_END_PACKAGE2()
