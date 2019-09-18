#include "Precompiled.h"
#include "ts/tessa/resource/ImageResource.h"

#include "ts/tessa/system/Gigaton.h"
#include "ts/tessa/file/ArchivistFilesystem.h"
#include "ts/tessa/resource/ArchivistInputStream.h"

TS_DEFINE_RESOURCE_TYPE(resource::ImageResource);

TS_PACKAGE1(resource)

ImageResource::ImageResource(const String &filepath)
	: ResourceBase(filepath)
{
}

ImageResource::~ImageResource()
{
}

bool ImageResource::loadResourceImpl()
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
				if (resource->loadFromStream(strm))
				{
					return true;
				}
				else
				{
					TS_LOG_ERROR("Failed to load a texture from archivist input stream. File: %s", filepath);
				}
			}
			else
			{
				TS_ASSERT(false);
			}
		}

		// Fallback from normal file system
		if (resource->loadFromFile(getAbsolutePath()))
		{
			return true;
		}
		else
		{
			TS_LOG_ERROR("Failed to load a texture from disk. File: %s", filepath);
		}

		return false;
	})();

	if (!success)
		return false;

	return true;
}

TS_END_PACKAGE1()
