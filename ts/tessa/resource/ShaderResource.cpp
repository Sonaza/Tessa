#include "Precompiled.h"
#include "ts/tessa/resource/ShaderResource.h"
#include <szen/System/ErrorStream.hpp>

#include <szen/System/Window.hpp>

TS_PACKAGE1(resource)

ShaderResource::ShaderResource(const std::string &filepath) :
	ResourceBase(path)
{
}

ShaderResource::~ShaderResource()
{
}

bool ShaderResource::loadResource()
{
	TS_ASSERT(!m_loaded && "Shader has already been loaded");
	TS_ASSERT(!m_filename.empty() && "Must have file name");

	m_asset = new(std::nothrow) sf::Shader();
	TS_ASSERT(m_asset && "Allocation failed");

	if(!m_asset->loadFromFile(m_filename, sf::Shader::Fragment))
	{
		szerr << "Unable to open shader file: " << m_filename << ErrorStream::error;

		return false;
	}

	m_asset->setParameter("u_resolution", static_cast<sf::Vector2f>(Window::getSize()));
	m_asset->setParameter("u_vresolution", static_cast<sf::Vector2f>(Window::getVirtualSize()));

	m_asset->setParameter("u_texture", sf::Shader::CurrentTexture);

	m_loaded = true;

	return true;
}

TS_END_PACKAGE1()
