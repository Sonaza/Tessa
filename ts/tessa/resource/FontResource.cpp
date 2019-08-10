#include "Precompiled.h"
#include "ts/tessa/resource/FontResource.h"

#include "ts/tessa/system/Gigaton.h"
#include "ts/tessa/file/ArchivistFilesystem.h"
#include "ts/tessa/resource/ArchivistInputStream.h"

TS_DEFINE_RESOURCE_TYPE(resource::FontResource);

TS_PACKAGE1(resource)

FontResource::FontResource(const std::string &filepath)
	: ResourceBase(filepath)
{
	
}

FontResource::~FontResource()
{
	if (strm != nullptr)
		strm.reset();
}

bool FontResource::loadResourceImpl()
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
				// Font continues to use the stream on demand, so allocate a longer lasting instance of it
				strm.reset(new ArchivistInputStream(extractor));
				if (resource->loadFromStream(*strm))
				{
					return true;
				}
				else
				{
					TS_LOG_ERROR("Failed to load the font from archivist input stream. File: %s", filepath);
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
			TS_LOG_ERROR("Failed to load the font from disk. File: %s", filepath);
		}

		return false;
	})();

	return success;
}

TS_END_PACKAGE1()
