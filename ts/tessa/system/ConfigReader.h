#pragma once

#include <string>
#include <set>
#include <unordered_map>

TS_PACKAGE1(system)

class ConfigReader : public lang::Noncopyable
{
public:
	ConfigReader();
	explicit ConfigReader(const std::string &configFilePath);
	~ConfigReader();

	// Opens specified config file and parses it.
	bool open(const std::string &configFilePath);

	// Saves current configuration to the previously opened config file.
	bool save();
	bool save(const std::string &configFilePath);

	void clear();

	const std::set<std::string> &getSections() const;
	const std::set<std::string> &getKeys(const std::string &section);

	const std::string &getString(const std::string &section_dot_key, const std::string &defaultValue) const;
	void setString(const std::string &section_dot_key, const std::string &value);

	Int32 getInt32(const std::string &section_dot_key, const Int32 defaultValue) const;
	void setInt32(const std::string &section_dot_key, const Int32 value);

	Uint32 getUint32(const std::string &section_dot_key, const Uint32 defaultValue) const;
	void setUint32(const std::string &section_dot_key, const Uint32 value);

	float getFloat(const std::string &section_dot_key, const float defaultValue) const;
	void setFloat(const std::string &section_dot_key, const float value, Int32 precision = 3);

	bool getBoolean(const std::string &section_dot_key, const bool defaultValue) const;
	void setBoolean(const std::string &section_dot_key, const bool value);

protected:
	static int parserHandler(void *userdata, const char *section, const char *key, const char *value);
	static Uint32 makeHash(const std::string &token);

	void setValue(const std::string &section_dot_key, const std::string &value);

	bool parseSectionAndKey(const std::string &section_dot_key, std::string &outSection, std::string &outKey);

	std::set<std::string> sections;
	std::unordered_map<Uint32, std::set<std::string>> sectionKeys;
	std::unordered_map<Uint32, std::string> values;

	static const std::set<std::string> emptySet;

	std::string filepath;
};


TS_END_PACKAGE1()
