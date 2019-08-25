#include "Precompiled.h"
#include "ts/tessa/resource/AbstractResourceBase.h"

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
	return ResourceManager::getAbsoluteResourcePath(filepath);
}

TS_END_PACKAGE1()


