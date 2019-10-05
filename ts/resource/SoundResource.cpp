#include "Precompiled.h"
#include "ts/resource/SoundResource.h"

#include "ts/engine/Gigaton.h"
#include "ts/resource/archivist/ArchivistFilesystem.h"
#include "ts/resource/archivist/ArchivistInputStream.h"

TS_DEFINE_RESOURCE_TYPE(resource::SoundResource);

TS_PACKAGE1(resource)

SoundResource::SoundResource(const String &filepath)
	: ResourceBase(filepath)
{
}

SoundResource::~SoundResource()
{
}

bool SoundResource::loadResourceImpl()
{
	bool success = ([&]()
	{
		archivist::ArchivistFilesystem &afs = TS_GET_GIGATON().getGigaton<archivist::ArchivistFilesystem>();

		// First try if it exists in the virtual file system
		if (afs.fileExists(filepath))
		{
			archivist::ArchivistReaderExtractor extractor;
			if (afs.getFileExtractor(filepath, extractor))
			{
				archivist::ArchivistInputStream strm(extractor);
				if (resource->loadFromStream(strm))
				{
					return true;
				}
				else
				{
					TS_LOG_ERROR("Failed to load a sound buffer from archivist input stream. File: %s", filepath);
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
			TS_LOG_ERROR("Failed to load a sound buffer from disk. File: %s", filepath);
		}

		return false;
	})();

	return success;
}

TS_END_PACKAGE1()
