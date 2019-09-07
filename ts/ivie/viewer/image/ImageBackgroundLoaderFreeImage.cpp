#include "Precompiled.h"
#include "ImageBackgroundLoaderFreeImage.h"

#include "ts/tessa/thread/Thread.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/ivie/viewer/image/Image.h"
#include "ts/ivie/util/RenderUtil.h"

#include "ts/tessa/profiling/ZoneProfiler.h"

TS_PACKAGE2(app, viewer)

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
	state.memoryBuffer.resize(filesize);
	fileHandle.read(&state.memoryBuffer[0], filesize);
	fileHandle.close();

	state.memory = FreeImage_OpenMemory(&state.memoryBuffer[0], (DWORD)filesize);
	if (state.memory == nullptr)
	{
		TS_WLOG_ERROR("FreeImage_OpenMemory failed. File: %s\n", filepath);
		errorText = "FreeImage_OpenMemory failed";
		return false;
	}

	state.format = FreeImage_GetFileTypeFromMemory(state.memory);
// 	if (state.format == FIF_UNKNOWN)
// 	{
// 		// Try to get file type from file name instead
// 		state.format = FreeImage_GetFIFFromFilenameU(filepath.c_str());
// 	}

	int32 flags = 0;

	switch (state.format)
	{
		case FIF_UNKNOWN:
		{
			errorText = "Unknown or unsupported format.";
			return false;
		}

		case FIF_GIF:
// 		case FIF_MNG:
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

	SizeType maxSize = sf::Texture::getMaximumSize();
	if (imageSize.x > maxSize || imageSize.y > maxSize)
	{
		TS_LOG_ERROR("Image is too large.");
		errorText = "Image is too large.";
		return false;
	}

	if (FreeImage_GetBPP(state.bitmap) != 32)
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
	imageData.hasAlpha = true;

	bool success = false;

	bufferStorage.texture = makeShared<sf::Texture>();

	if (bufferStorage.texture != nullptr && bufferStorage.texture->create(imageSize.x, imageSize.y))
	{
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

bool ImageBackgroundLoaderFreeImage::isValidFreeImageFile(const String &filepath)
{
#if TS_PLATFORM == TS_WINDOWS
	FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeU(filepath.toWideString().c_str());
#else
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filepath.toUtf8().c_str());
#endif
// 	if (format == FIF_UNKNOWN)
// 	{
// 		// Try to get file type from file name instead
// 		format = FreeImage_GetFIFFromFilenameU(filepath.c_str());
// 	}
	return FreeImage_FIFSupportsReading(format) == 1;
}

bool ImageBackgroundLoaderFreeImage::restartImpl(bool *shouldRestart)
{
	TS_ASSERT(shouldRestart != nullptr);

	// Only restart for multibitmaps
	if (loaderFormat == MultiBitmapFormat)
	{
		loaderIsComplete = false;
		currentPage = 0;

		*shouldRestart = true;
	}
	else
	{
		*shouldRestart = false;
	}

	return true;
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


