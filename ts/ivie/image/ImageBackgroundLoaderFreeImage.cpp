#include "Precompiled.h"
#include "ImageBackgroundLoaderFreeImage.h"

#include "ts/thread/Thread.h"
#include "ts/file/FileUtils.h"
#include "ts/profiling/ZoneProfiler.h"
#include "ts/ivie/image/Image.h"
#include "ts/ivie/util/RenderUtil.h"

#include <set>

TS_PACKAGE2(app, image)

static const std::map<FREE_IMAGE_FORMAT, bool> FormatAlphaSupport
{
	// TEEEECHNICALLY BMP does have alpha but I haven't ever seen it actually being used and now it's also causing problems.
	{ FIF_BMP, false, }, 
	{ FIF_CUT, true, },
	{ FIF_DDS, true, },
	{ FIF_EXR, true, },
	{ FIF_FAXG3, true, },
	{ FIF_GIF, true, },
	{ FIF_HDR, true, },
	{ FIF_ICO, true, },
	{ FIF_J2K, true, },
	{ FIF_JNG, false, },
	{ FIF_JP2, true, },
	{ FIF_JPEG, false, },
	{ FIF_JXR, true, },
	{ FIF_KOALA, true, },
	{ FIF_LBM, true, },
	{ FIF_MNG, true, },
	{ FIF_PBM, true, },
	{ FIF_PBMRAW, true, },
	{ FIF_PCD, true, },
	{ FIF_PCX, true, },
	{ FIF_PFM, true, },
	{ FIF_PGM, true, },
	{ FIF_PGMRAW, true, },
	{ FIF_PICT, true, },
	{ FIF_PNG, true, },
	{ FIF_PPM, true, },
	{ FIF_PPMRAW, true, },
	{ FIF_PSD, true, },
	{ FIF_RAS, true, },
	{ FIF_RAW, true, },
	{ FIF_SGI, true, },
	{ FIF_TARGA, true, },
	{ FIF_TIFF, true, },
	{ FIF_WBMP, true, },
	{ FIF_WEBP, true, },
	{ FIF_XBM, true, },
	{ FIF_XPM, true, },
};

ImageBackgroundLoaderFreeImage::ImageBackgroundLoaderFreeImage(Image *ownerImage, const String &filepath)
	: AbstractImageBackgroundLoader(ownerImage, filepath)
{
	TS_ZONE();
}

ImageBackgroundLoaderFreeImage::~ImageBackgroundLoaderFreeImage()
{
// 	TS_WPRINTF("~ImageBackgroundLoaderFreeImage()  : Task ID %u [%s]\n", taskId, filepath);

	TS_ASSERTF(loaderIsPrepared == false, "Cleanup is incomplete. Task ID %u", taskId);
	TS_ASSERT(stackingRenderTextureThreadId == -1 && "It should've been already freed.");
	TS_ASSERT(stackingRenderTexture == nullptr && "Render texture must be freed by the thread that created it.");
}

bool ImageBackgroundLoaderFreeImage::initialize()
{
	return true;
}

void ImageBackgroundLoaderFreeImage::deinitialize()
{
// 	TS_WPRINTF("ImageBackgroundLoaderFreeImage::deinitialize()  : Task ID %u [%s]\n", taskId, filepath);
	cleanup();
}

void ImageBackgroundLoaderFreeImage::onResume()
{
	if (stackingRenderTexture == nullptr && savedRenderTexture != nullptr)
	{
		stackingRenderTexture = makeUnique<sf::RenderTexture>();
		stackingRenderTexture->create(imageSize.x, imageSize.y);
		stackingRenderTexture->draw(imageVertexArray, savedRenderTexture.get());
		savedRenderTexture.reset();

		stackingRenderTextureThreadId = (int32)Thread::getCurrentThread().getThreadId();
	}
}

void ImageBackgroundLoaderFreeImage::onSuspend()
{
	if (stackingRenderTexture != nullptr)
	{
		int32 threadId = (int32)Thread::getCurrentThread().getThreadId();
		TS_ASSERTF(stackingRenderTextureThreadId == threadId,
			"Thread ID mismatch. Got: %d  Expected: %d", threadId, stackingRenderTextureThreadId);

		// Store render texture before suspension since its GL context requires that the 
		// creation and deletion happen in the same thread. Gotta deinit before suspending.
		savedRenderTexture = makeShared<sf::Texture>(stackingRenderTexture->getTexture());
		stackingRenderTexture.reset();
		stackingRenderTextureThreadId = -1;
	}
}

bool ImageBackgroundLoaderFreeImage::prepareForLoading()
{
	TS_ZONE();

	TS_ASSERT(loaderIsPrepared == false && "Loader is already prepared.");

	file::InputFile fileHandle;
	if (!fileHandle.open(filepath, file::InputFileMode_ReadBinary))
	{
		TS_WLOG_ERROR("Failed to open file. File: %s\n", filepath);
		errorText = "Failed to open file. File doesn't exist?";
		return false;
	}

	PosType filesize = fileHandle.getSize();
	TS_ASSERT(filesize > 0);

	state.memoryBuffer.resize(filesize);
	fileHandle.read(&state.memoryBuffer[0], (SizeType)filesize);
	fileHandle.close();

	state.memory = FreeImage_OpenMemory(&state.memoryBuffer[0], (DWORD)filesize);
	if (state.memory == nullptr)
	{
		TS_WLOG_ERROR("FreeImage_OpenMemory failed. File: %s\n", filepath);
		errorText = "FreeImage_OpenMemory failed";
		return false;
	}

	state.format = FreeImage_GetFileTypeFromMemory(state.memory);

	imageData.canBeRotated = isValidRotateFormat(state.format);

	int32 flags = 0;

	switch (state.format)
	{
		case FIF_UNKNOWN:
		{
			errorText = "Unknown or unsupported format.";
			return false;
		}

		case FIF_GIF:
		case FIF_MNG:
		{
			loaderFormat = MultiBitmapFormat;

			state.multibitmap = FreeImage_LoadMultiBitmapFromMemory(state.format, state.memory, flags);
			if (state.multibitmap == nullptr)
			{
				TS_WLOG_ERROR("FreeImage_LoadMultiBitmapFromMemory failed. File: %s", filepath);
				errorText = "Unknown or unsupported format.";
				return false;
			}

			numPagesTotal = FreeImage_GetPageCount(state.multibitmap);
		}
		break;

		default:
		{
			loaderFormat = StillImageFormat;

			switch (state.format)
			{
				case FIF_ICO:
				{
					flags |= ICO_MAKEALPHA;
				}
				break;

				case FIF_JPEG:
				{
					flags |= JPEG_EXIFROTATE | JPEG_ACCURATE;
				}
				break;
				
				default: break;
			}

			state.bitmap = FreeImage_LoadFromMemory(state.format, state.memory, flags);
			if (state.bitmap == nullptr)
			{
				TS_WLOG_ERROR("FreeImage_LoadFromMemory failed. File: %s", filepath);
				errorText = "Unknown or unsupported format.";
				return false;
			}
		}
		break;
	}

	loaderIsPrepared = true;

	return true;
}

void ImageBackgroundLoaderFreeImage::cleanup(bool soft)
{
// 	TS_WPRINTF("ImageBackgroundLoaderFreeImage::cleanup()  : Task ID %u [%s]\n", taskId, filepath);

	switch (loaderFormat)
	{
		case StillImageFormat:
		{
			if (state.bitmap != nullptr)
			{
				FreeImage_Unload(state.bitmap);
				state.bitmap = nullptr;
			}
		}
		break;

		case MultiBitmapFormat:
		{
			if (state.multibitmap != nullptr)
			{
				FreeImage_CloseMultiBitmap(state.multibitmap);
				state.multibitmap = nullptr;
			}
		}
		break;
		
		default: TS_ASSERT(!"Unhandled format"); break;
	}

	state.format = FIF_UNKNOWN;

	if (state.memory != nullptr)
	{
		FreeImage_CloseMemory(state.memory);
		state.memory = nullptr;
	}
	state.memoryBuffer.clear();

	previousFrame.reset();

	if (stackingRenderTexture)
	{
		int32 threadId = (int32)Thread::getCurrentThread().getThreadId();
		TS_ASSERTF(stackingRenderTextureThreadId == threadId,
			"Thread ID mismatch. Got: %d  Expected: %d", threadId, stackingRenderTextureThreadId);

		stackingRenderTexture.reset();
		savedRenderTexture.reset();
		stackingRenderTextureThreadId = -1;
	}

	if (soft == false)
	{
		loaderIsPrepared = false;
		loaderIsComplete = false;
	}

// 	TS_PRINTF("Cleanup complete.\n");
}

bool ImageBackgroundLoaderFreeImage::processNextStill(FrameStorage &bufferStorage)
{
	TS_ASSERT(state.bitmap != nullptr);

	imageSize.x = FreeImage_GetWidth(state.bitmap);
	imageSize.y = FreeImage_GetHeight(state.bitmap);

// 	FIICCPROFILE *icc = FreeImage_GetICCProfile(state.bitmap);
// 	if (icc->size > 0)
// 	{
// 		TS_WPRINTF("%s has some profile? %u bytes\n", filepath, icc->size);
// 	}
// 	else
// 	{
// 		TS_WPRINTF("%s does not have profile?\n", filepath);
// 	}

	uint32 maxSize = sf::Texture::getMaximumSize();
	if (imageSize.x > maxSize || imageSize.y > maxSize)
	{
		TS_LOG_ERROR("Image is too large.");
		errorText = "Image is too large.";
		return false;
	}

	FREE_IMAGE_COLOR_TYPE originalColorType = FreeImage_GetColorType(state.bitmap);

	uint32 bitdepth = FreeImage_GetBPP(state.bitmap);
	if (bitdepth != 32)
	{
		FIBITMAP *temp = FreeImage_ConvertTo32Bits(state.bitmap);

		// Unload previous image and replace it with the new one
		FreeImage_Unload(state.bitmap);
		state.bitmap = temp;
	}

	BYTE *bits = FreeImage_GetBits(state.bitmap);
	if (bits == nullptr)
	{
		TS_LOG_ERROR("FreeImage_GetBits returned null.");
		errorText = "FreeImage_GetBits failed.";
		return false;
	}

	imageData.size = imageSize;
	
	if (originalColorType != FIC_RGBALPHA)
	{
		imageData.hasAlpha = false;
	}
	else if (FormatAlphaSupport.count(state.format) > 0)
	{
		imageData.hasAlpha = FormatAlphaSupport.at(state.format);
	}
	else
	{
		imageData.hasAlpha = true;
	}

	imageData.numFramesTotal = 1;

	bool success = false;

	bufferStorage.texture = makeShared<sf::Texture>();

	if (bufferStorage.texture != nullptr && bufferStorage.texture->create(imageSize.x, imageSize.y))
	{
		if (!imageData.hasAlpha)
		{
			BYTE *pixel = bits;
			for (SizeType y = 0; y < imageSize.y; ++y)
			{
				for (SizeType x = 0; x < imageSize.x; ++x)
				{
					pixel[FI_RGBA_ALPHA] = 255U;
					pixel += 4;
				}
			}
		}

		bufferStorage.texture->update(bits, imageSize.x, imageSize.y, 0, 0, sf::Texture::BGRA);

		if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			bufferStorage.texture->generateMipmap();

// 		bufferStorage.texture->setSmooth(true);

		loaderIsComplete = true;

		success = true;
	}

	cleanup(true);
	
	return success;
}

bool ImageBackgroundLoaderFreeImage::processNextMultiBitmap(FrameStorage &bufferStorage)
{
	TS_ASSERT(state.multibitmap != nullptr);

	FIBITMAP *lockedPage = FreeImage_LockPage(state.multibitmap, currentPage);
	if (lockedPage == nullptr)
	{
		TS_LOG_ERROR("Failed to lock multibitmap page index %u", currentPage);
		errorText = "Failed to lock multibitmap page.";
		return false;
	}

	math::VC2U frameSize = math::VC2U(
		FreeImage_GetWidth(lockedPage),
		FreeImage_GetHeight(lockedPage)
	);

	if (currentPage == 0)
	{
		if (multibitmapInitialized == false)
		{
			imageSize = frameSize;

			imageData.size = imageSize;
			imageData.hasAlpha = (FreeImage_IsTransparent(lockedPage) == 1);
			imageData.numFramesTotal = numPagesTotal;

			stackingRenderTexture = makeUnique<sf::RenderTexture>();
			stackingRenderTexture->create(imageSize.x, imageSize.y);
			stackingRenderTextureThreadId = (int32)Thread::getCurrentThread().getThreadId();

			imageVertexArray = util::makeQuadVertexArray(imageSize.x, imageSize.y);

			multibitmapInitialized = true;
		}

		stackingRenderTexture->clear(sf::Color::Transparent);
		lastDisposalMethod = DisposalMethod_NotSet;
		previousFrame.reset();
	}

	FITAG *tag = nullptr;

	uint32 frametime = 100;
	if (FreeImage_GetMetadata(FIMD_ANIMATION, lockedPage, "FrameTime", &tag))
	{
		frametime = *(uint32*)FreeImage_GetTagValue(tag);

		// Use 100ms as a default if a proper value wasn't stored
		if (frametime == 0)
			frametime = 100;
	}
	bufferStorage.frameTime = TimeSpan::fromMilliseconds(frametime);

	math::VC2U offset;
	if (FreeImage_GetMetadata(FIMD_ANIMATION, lockedPage, "FrameLeft", &tag))
		offset.x = *(uint16_t*)FreeImage_GetTagValue(tag);

	if (FreeImage_GetMetadata(FIMD_ANIMATION, lockedPage, "FrameTop", &tag))
		offset.y = *(uint16_t*)FreeImage_GetTagValue(tag);

	DisposalMethod disposalMethod = DisposalMethod_Leave;
	if (FreeImage_GetMetadata(FIMD_ANIMATION, lockedPage, "DisposalMethod", &tag))
		disposalMethod = *(DisposalMethod*)FreeImage_GetTagValue(tag);

	FIBITMAP *bitmap32bpp = lockedPage;

	if (FreeImage_GetBPP(bitmap32bpp) != 32)
		bitmap32bpp = FreeImage_ConvertTo32Bits(bitmap32bpp);

	TS_ASSERT(bitmap32bpp != nullptr);

	BYTE *bits = FreeImage_GetBits(bitmap32bpp);
	if (bits == nullptr)
	{
		TS_LOG_ERROR("FreeImage_GetBits returned null.");
		errorText = "FreeImage_GetBits failed.";
		return false;
	}

	bool success = false;

	sf::Texture currentFrame;
	if (currentFrame.create(frameSize.x, frameSize.y))
	{
		currentFrame.update(bits, sf::Texture::BGRA);

		if (disposalMethod == DisposalMethod_Previous && previousFrame != nullptr)
		{
			stackingRenderTexture->draw(imageVertexArray, previousFrame.get());
		}
		else if (disposalMethod == DisposalMethod_Background)
		{
			stackingRenderTexture->clear(sf::Color::Transparent);
		}

		const math::VC2U sizeDiff = imageSize - frameSize;
		stackingRenderTexture->draw(
			util::makeQuadVertexArray(frameSize.x, frameSize.y, offset.x, sizeDiff.y - offset.y),
			&currentFrame);
		stackingRenderTexture->display();

		const sf::Texture &stackedTexture = stackingRenderTexture->getTexture();

		bufferStorage.texture = makeShared<sf::Texture>(stackedTexture);
		if (bufferStorage.texture != nullptr)
		{
			if (disposalMethod != DisposalMethod_Previous)
				previousFrame = bufferStorage.texture;

			bufferStorage.texture->setSmooth(true);

			if (currentPage + 1 == numPagesTotal && numPagesTotal < ownerImage->frameBuffer.getMaxSize())
				loaderIsComplete = true;

			success = true;
		}
	}

	if (bitmap32bpp != lockedPage)
		FreeImage_Unload(bitmap32bpp);

	FreeImage_UnlockPage(state.multibitmap, lockedPage, FALSE);

	currentPage = (currentPage + 1) % numPagesTotal;

	return success;
}

bool ImageBackgroundLoaderFreeImage::isLoadingComplete() const
{
	return BaseClass::isLoadingComplete() && loaderIsComplete;
}

static FREE_IMAGE_FORMAT getFreeImageFormatForFile(const String &filepath)
{
#if TS_PLATFORM == TS_WINDOWS
	FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeU(filepath.toWideString().c_str());
#else
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filepath.toUtf8().c_str());
#endif
	return format;
}

bool ImageBackgroundLoaderFreeImage::isValidFreeImageFile(const String &filepath)
{
	FREE_IMAGE_FORMAT format = getFreeImageFormatForFile(filepath);
	return FreeImage_FIFSupportsReading(format) == 1;
}

bool ImageBackgroundLoaderFreeImage::canImageBeRotated(const String &filepath)
{
	FREE_IMAGE_FORMAT format = getFreeImageFormatForFile(filepath);
	return isValidRotateFormat(format);
}

bool ImageBackgroundLoaderFreeImage::isValidRotateFormat(FREE_IMAGE_FORMAT format)
{
	if (format == FIF_UNKNOWN)
		return false;

	const std::set<FREE_IMAGE_FORMAT> validRotateFormats{
		FIF_BMP, FIF_JPEG, FIF_PNG, FIF_WEBP, FIF_TARGA, FIF_TIFF,
	};

	return validRotateFormats.count(format) > 0;
}

static bool transformJPEG(const String &srcFile, const String &dstFile, FREE_IMAGE_JPEG_OPERATION operation)
{
#if TS_PLATFORM == TS_WINDOWS
	return FreeImage_JPEGTransformU(srcFile.toWideString().c_str(), dstFile.toWideString().c_str(), operation, FALSE) == TRUE;
#else
	return FreeImage_JPEGTransform(srcFile.toUtf8().c_str(), dstFile.toUtf8().c_str(), operation, FALSE) == TRUE;
#endif
}

bool ImageBackgroundLoaderFreeImage::rotate(const String &filepath, int32 direction)
{
	TS_ASSERTF(direction == 1 || direction == -1, "Invalid direction, must be 1 or -1");
	if (direction != 1 && direction != -1)
		return false;

	FREE_IMAGE_FORMAT format = getFreeImageFormatForFile(filepath);
	bool validFormat = isValidRotateFormat(format);
	
	TS_PRINTF("Rotate: %s\n", filepath);
	TS_PRINTF("  Valid format: %s\n", validFormat ? "yes" : "no");

	if (format == FIF_JPEG)
	{
		TS_PRINTF("ASDASD!\n");

		String dstFile = filepath;
		dstFile.replace(".jpg", "_rotated.jpg");

		if (direction > 0)
			return transformJPEG(filepath, dstFile, FIJPEG_OP_ROTATE_90);
		else
			return transformJPEG(filepath, dstFile, FIJPEG_OP_ROTATE_270);
	}
	else
	{
		TS_PRINTF("HELLO!\n");
	}

	return true;
}

int32 ImageBackgroundLoaderFreeImage::restartImpl()
{
	// Only restart for multibitmaps
	if (loaderFormat == MultiBitmapFormat)
	{
		loaderIsComplete = false;
		currentPage = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

bool ImageBackgroundLoaderFreeImage::loadNextFrame(FrameStorage &bufferStorage)
{
	TS_ASSERT(loaderIsComplete == false && "Is already complete.");
	if (loaderIsComplete || loaderState != Running)
		return false;

	if (!loaderIsPrepared)
	{
		bool success = prepareForLoading();
		if (!success)
		{
			cleanup();
			return false;
		}
	}

	switch (loaderFormat)
	{
		case StillImageFormat:
		{
			bool success = processNextStill(bufferStorage);
			if (!success)
				return false;
		}
		break;

		case MultiBitmapFormat:
		{
			bool success = processNextMultiBitmap(bufferStorage);
			if (!success)
				return false;
		}
		break;
		
		default: TS_ASSERT(!"Unhandled format"); break;
	}

	if (imageDataUpdated == false)
	{
		TS_ASSERT(imageData.size.x > 0 && imageData.size.y > 0 && "Image size is not set.");
		ownerImage->setImageData(imageData);
		imageDataUpdated = true;
	}

	return true;
}

bool ImageBackgroundLoaderFreeImage::wasLoadingCompleted() const
{
	return loaderIsComplete;
}

TS_END_PACKAGE2()

