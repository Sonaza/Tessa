#include "Precompiled.h"
#include "Image.h"

#include "ts/tessa/file/InputFile.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/ivie/viewer/image/ImageBackgroundLoaderFreeImage.h"
// #include "ts/ivie/viewer/image/ImageBackgroundLoaderWebm.h"

#include "FreeImage.h"
// #include "nestegg/nestegg.h"

TS_PACKAGE2(app, viewer)

Image::Image(const std::wstring &filepath)
	: filepath(filepath)
{
	TS_ASSERT(!filepath.empty());
}

Image::~Image()
{
	backgroundLoader.reset();
}

bool Image::startLoading(bool suspendAfterBufferFull)
{
	TS_ASSERT(loaderState == Unloaded);
	if (loaderState != Unloaded)
		return false;

	TS_WPRINTF("Starting to load file %s\n", filepath);

	std::unique_lock<std::mutex> lock(mutex);

	LoaderType type = sniffLoaderType();
	switch (type)
	{
		case LoaderFreeImage:
		{
			loaderState = Loading;
			backgroundLoader.reset(new ImageBackgroundLoaderFreeImage(this, filepath));
		}
		break;

		case LoaderWebm:
		{
			loaderState = Loading;
// 			backgroundLoader.reset(new ImageBackgroundLoaderWebm(this, filepath));
		}
		break;

		case LoaderUnknown:
		{
			loaderState = Error;
			TS_WLOG_ERROR("Unable to load file, unknown or unsupported type. File: %s", filepath);
			
			return false;
		}
		break;
	}

	TS_ASSERT(backgroundLoader);
	backgroundLoader->start(suspendAfterBufferFull);

	return true;
}

void Image::unload()
{
	std::unique_lock<std::mutex> lock(mutex);
	if (loaderState == Unloaded)
		return;

	backgroundLoader->stop();
	backgroundLoader.reset();

	frameBuffer.clear();

	data = ImageData();
	currentFrameIndex = 0;
	loaderState = Unloaded;
}

void Image::restart(bool suspend)
{
	std::unique_lock<std::mutex> lock(mutex);
	if (loaderState == Error)
		return;

	TS_ASSERT(backgroundLoader);

	if (backgroundLoader->restart(true))
	{
		currentFrameIndex = 0;
		frameBuffer.clear();
	}
}

bool Image::isUnloaded() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return loaderState == Unloaded;
}

bool Image::isSuspended() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return loaderState == Suspended && backgroundLoader && backgroundLoader->isSuspended();
}

void Image::resumeLoading()
{
	std::unique_lock<std::mutex> lock(mutex);
	TS_ASSERT(loaderState == Suspended);
	if (loaderState != Suspended)
		return;

	TS_ASSERT(backgroundLoader);
	backgroundLoader->resume();
}

const math::VC2U Image::getSize() const
{
	std::unique_lock<std::mutex> lock(mutex);
	TS_ASSERT(data.size.x > 0 && data.size.y > 0 && "Image size is not set.");
	return data.size;
}

const FrameStorage *Image::getCurrentFrameStorage() const
{
	std::unique_lock<std::mutex> lock(mutex);
	if (!frameBuffer.isEmpty())
		return &frameBuffer.getReadPtr();
	return nullptr;
}

SizeType Image::getCurrentFrameIndex() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return currentFrameIndex;
}

SizeType Image::getNumFramesTotal() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return data.numFramesTotal;
}

SizeType Image::getNumFramesBuffered() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return (SizeType)frameBuffer.getBufferedAmount();
}

bool Image::getIsAnimated() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return data.numFramesTotal > 1;
}

bool Image::getHasAlpha() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return data.hasAlpha;
}

bool Image::advanceToNextFrame()
{
	static SizeType lastFrameIndex = ~0U;

	std::unique_lock<std::mutex> lock(mutex);

	backgroundLoader->requestNextFrame();

	if (loaderState == Complete || frameBuffer.canIncrementRead())
	{
		frameBuffer.incrementRead();
		currentFrameIndex = (currentFrameIndex + 1) % data.numFramesTotal;
		return true;
	}
	return false;
}

bool Image::isDisplayable() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return loaderState != Error && !frameBuffer.isEmpty();
}

bool Image::hasError() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return loaderState == Error;
}

void Image::setImageData(const ImageData &dataParam)
{
	std::unique_lock<std::mutex> lock(mutex);
	data = dataParam;
}

Image::LoaderType Image::sniffLoaderType()
{
	// Test FreeImage formats
	{
		FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeU(filepath.c_str());
		if (format == FIF_UNKNOWN)
		{
			// Try to get file type from file name instead
			format = FreeImage_GetFIFFromFilenameU(filepath.c_str());
		}

		if (FreeImage_FIFSupportsReading(format))
			return LoaderFreeImage;
	}

	// Test Nestegg format (webm)
	/*{
		static const BigSizeType nesteggSniffBytesAmount = 512;

		file::InputFile file(filepath, file::InputFileMode_ReadBinary);

		unsigned char frameBuffer[nesteggSniffBytesAmount] = { 0 };
		file.read(&frameBuffer[0], nesteggSniffBytesAmount);
		file.close();

		if (nestegg_sniff(&frameBuffer[0], nesteggSniffBytesAmount) == 1)
			return LoaderWebm;
	}*/

	return LoaderUnknown;
}

Image::ImageLoaderState Image::getState() const
{
	std::unique_lock<std::mutex> lock(mutex);
	return loaderState;
}

const std::wstring &Image::getFilepath() const
{
	// No mutexing because value doesn't ever change after creation, reads should always be ok
	return filepath;
}

std::wstring Image::getStats() const
{
	std::unique_lock<std::mutex> lock(mutex);
	std::wstring str = TS_WFMT("%s (%u / %u [%u buffered]) Image: %s Loader: %s",
		file::utils::getBasename(filepath),
		currentFrameIndex + 1,
		math::max(1U, data.numFramesTotal),
		frameBuffer.getBufferedAmount(),
		getStateString(loaderState),
		backgroundLoader ? backgroundLoader->getStateString(backgroundLoader->getState()) : L"null"
	);
	return str;
}

void Image::setState(ImageLoaderState state)
{
	std::unique_lock<std::mutex> lock(mutex);
	loaderState = state;
}

TS_END_PACKAGE2()
