#include "Precompiled.h"
#include "ts/resource/MusicResource.h"

#include "ts/engine/Gigaton.h"
#include "ts/resource/archivist/ArchivistFilesystem.h"
#include "ts/resource/archivist/ArchivistInputStream.h"

TS_DEFINE_RESOURCE_TYPE(resource::MusicResource);

TS_PACKAGE1(resource)

MusicResource::MusicResource(const String &filepath)
	: ResourceBase(filepath)
{
	
}

MusicResource::~MusicResource()
{
	if (strm != nullptr)
		strm.reset();
}

bool MusicResource::loadResourceImpl()
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
				// sf::Music continues to use the stream on demand, so allocate a longer lasting instance of it
				strm.reset(new archivist::ArchivistInputStream(extractor));
				if (resource->openFromStream(*strm))
				{
					return true;
				}
				else
				{
					TS_LOG_ERROR("Failed to open the music source from archivist input stream. File: %s", filepath);
				}
			}
			else
			{
				TS_ASSERT(false);
			}
		}

		// Fallback from normal file system
		if (resource->openFromFile(getAbsolutePath()))
		{
			return true;
		}
		else
		{
			TS_LOG_ERROR("Failed to open the music source from disk. File: %s", filepath);
		}

		return false;
	})();

	return success;
}

TS_END_PACKAGE1()
