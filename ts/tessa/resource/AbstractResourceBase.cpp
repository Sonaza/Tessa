#include "Precompiled.h"
#include "ts/tessa/resource/AbstractResourceBase.h"

#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/resource/ResourceManager.h"

TS_PACKAGE1(resource)

AbstractResourceBase::AbstractResourceBase(const std::string &filepath)
	: filepath(filepath)
{

}

AbstractResourceBase::~AbstractResourceBase()
{

}

std::string AbstractResourceBase::getAbsolutePath()
{
	if (!file::utils::isAbsolutePath(filepath))
		return file::utils::joinPaths(ResourceManager::getResourceRootDirectory(), filepath);

	return filepath;
}

TS_END_PACKAGE1()


