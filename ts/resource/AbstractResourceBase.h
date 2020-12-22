#pragma once

#include "ts/engine/GUID.h"

TS_PACKAGE1(resource)

struct DataSourceGeneratedType { explicit DataSourceGeneratedType() = default; };
struct DataSourceFileType { explicit DataSourceFileType() = default; };

constexpr DataSourceGeneratedType DataSourceGenerated{};
constexpr DataSourceFileType DataSourceFile{};

class AbstractResourceBase
{
public:
	AbstractResourceBase(DataSourceGeneratedType t);
	AbstractResourceBase(DataSourceFileType t, const String &filepath);
	virtual ~AbstractResourceBase();

	virtual bool loadResource() = 0;
	virtual void unloadResource() = 0;

protected:
	enum class DataSource
	{
		Generated,
		LoadedFromFile,
	};

	String getAbsolutePath();
	static GUID generateRandomGuid();

	DataSource dataSource;
	String filepath;

private:
	// Make resource non-copyable
	AbstractResourceBase(const AbstractResourceBase &) = delete;
};

TS_END_PACKAGE1()
