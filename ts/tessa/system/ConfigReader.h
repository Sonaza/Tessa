#pragma once

#include <string>
#include <set>
#include <map>

TS_PACKAGE1(system)

class ConfigReader
{
public:
	ConfigReader();
	ConfigReader(const std::string &configfile);
	~ConfigReader();

	bool isLoaded() const;

	bool parse(const std::string &configfile);

	const std::set<std::string> &getSections() const;
	const std::set<std::string> &getFields(const std::string &section);

	const std::string &getString(const std::string &section, const std::string &key, const std::string &defaultValue) const;
	const std::string &getString(const std::string &section_dot_key, const std::string &defaultValue) const;

	Int32 getInt32(const std::string &section, const std::string &key, const Int32 defaultValue) const;
	Int32 getInt32(const std::string &section_dot_key, const Int32 defaultValue) const;

	Uint32 getUint32(const std::string &section, const std::string &key, const Uint32 defaultValue) const;
	Uint32 getUint32(const std::string &section_dot_key, const Uint32 defaultValue) const;

	float getFloat(const std::string &section, const std::string &key, const float defaultValue) const;
	float getFloat(const std::string &section_dot_key, const float defaultValue) const;

	bool getBoolean(const std::string &section, const std::string &key, const bool defaultValue) const;
	bool getBoolean(const std::string &section_dot_key, const bool defaultValue) const;


protected:
	static int parserHandler(void *userdata, const char *section, const char *key, const char *value);
	static Uint32 makeHash(const std::string &token);

	std::set<std::string> sections;
	std::map<std::string, std::set<std::string>> sectionFields;
	std::map<Uint32, std::string> values;

	static const std::set<std::string> emptySet;

	bool loaded = false;
	std::string filepath;
};

TS_END_PACKAGE1()
