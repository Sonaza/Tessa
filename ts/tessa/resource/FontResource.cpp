#include "Precompiled.h"
#include "ts/tessa/resource/FontResource.h"
#include <szen/System/ErrorStream.hpp>

TS_PACKAGE1(resource)

FontResource::FontResource(const std::string &filepath) :
	ResourceBase(path)
{
	
}

FontResource::~FontResource()
{
	
}

bool FontResource::loadResource()
{
	TS_ASSERT(!m_loaded && "Shader has already been loaded");
	TS_ASSERT(!m_filename.empty() && "Must have file name");

	m_asset = new(std::nothrow) sf::Font();
	TS_ASSERT(m_asset && "Allocation failed");

	if(!m_asset->loadFromFile(m_filename))
	{
		szerr << "Unable to open shader file: " << m_filename << ErrorStream::error;

		return false;
	}

	m_loaded = true;

	return true;
}

TS_END_PACKAGE1()
