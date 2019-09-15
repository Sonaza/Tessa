#include "Precompiled.h"
#include "ShaderResource.h"

#include "ts/tessa/system/Gigaton.h"
#include "ts/tessa/file/ArchivistFilesystem.h"
#include "ts/tessa/resource/ArchivistInputStream.h"

TS_DEFINE_RESOURCE_TYPE(resource::ShaderResource);

TS_PACKAGE1(resource)

ShaderResource::ShaderResource(const String &filepath)
	: ResourceBase(filepath)
{
}

ShaderResource::~ShaderResource()
{
}

bool ShaderResource::loadResourceImpl()
{
	bool success = ([&]()
	{
		file::ArchivistFilesystem &afs = TS_GET_GIGATON().getGigaton<file::ArchivistFilesystem>();

		// First try if it exists in the virtual file system
		if (afs.fileExists(filepath))
		{
			file::ArchivistReaderExtractor extractor;
			if (afs.getFileExtractor(filepath, extractor))
			{
				ArchivistInputStream strm(extractor);
				if (resource->loadFromStream(strm, sf::Shader::Fragment))
				{
					TS_WPRINTF("Loaded from archivist filesystem: %s\n", filepath);
					return true;
				}
				else
				{
					TS_LOG_ERROR("Failed to load a shader from archivist input stream. File: %s", filepath);
				}
			}
			else
			{
				TS_ASSERT(false);
			}
		}

		// Fallback from normal file system
		if (resource->loadFromFile(getAbsolutePath(), sf::Shader::Fragment))
		{
			TS_WPRINTF("Loaded from normal filesystem: %s\n", filepath);
			return true;
		}
		else
		{
			TS_LOG_ERROR("Failed to load a shader from disk. File: %s", filepath);
		}

		return false;
	})();

	if (!success)
		return false;

	return true;
}

TS_END_PACKAGE1()
