#include "Precompiled.h"
#include "ts/resource/AbstractResourceBase.h"

#include "ts/resource/ResourceManager.h"
#include "ts/math/RandomGenerator.h"

TS_PACKAGE1(resource)

AbstractResourceBase::AbstractResourceBase(DataSourceGeneratedType t)
	: dataSource(DataSource::Generated)
{

}

AbstractResourceBase::AbstractResourceBase(DataSourceFileType t, const String &filepath)
	: dataSource(DataSource::LoadedFromFile)
	, filepath(filepath)
{

}

AbstractResourceBase::~AbstractResourceBase()
{

}

String AbstractResourceBase::getAbsolutePath()
{
	if (dataSource == DataSource::LoadedFromFile)
		return ResourceManager::getAbsoluteResourcePath(filepath);
	
	return filepath;
}

GUID AbstractResourceBase::generateRandomGuid()
{
	return GUID(math::generateRandom32());
}

TS_END_PACKAGE1()


