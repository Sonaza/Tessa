#include "Precompiled.h"
#include "ts/resource/TextureResource.h"

#include "ts/engine/Gigaton.h"
#include "ts/resource/archivist/ArchivistFilesystem.h"
#include "ts/resource/archivist/ArchivistInputStream.h"

TS_DEFINE_RESOURCE_TYPE(resource::TextureResource);

TS_PACKAGE1(resource)

TextureResource::TextureResource(const String &filepath)
	: ResourceBase(filepath)
{
}

TextureResource::TextureResource(const math::VC2U &size, const uint8_t *pixelData)
{
	TS_ASSERT(size.x > 0 && size.y > 0);
	TS_ASSERT(pixelData != nullptr);

	resourceLoaded = false;

	resource = makeShared<InternalResourceType>();
	if (resource == nullptr)
	{
		TS_LOG_ERROR("Failed to allocate memory for resource container.");
		loadError = true;
		return;
	}

	sf::Image image;
	image.create(size.x, size.y, pixelData);

	if (resource->loadFromImage(image))
	{
		resource->setSmooth(true);
		resource->generateMipmap();

		resourceLoaded = true;
		loadError = false;
	}
	else
	{
		TS_LOG_ERROR("Failed to create a texture from pixels.");
		loadError = true;
	}
}

TextureResource::~TextureResource()
{
}

void TextureResource::setSmooth(const bool enabledParam)
{
	if (resource != nullptr)
		return;

	resource->setSmooth(enabledParam);
}

bool TextureResource::loadResourceImpl()
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

	resource->setSmooth(true);
	resource->generateMipmap();

	return true;
}

TS_END_PACKAGE1()
