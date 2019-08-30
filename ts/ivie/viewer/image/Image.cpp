#include "Precompiled.h"
#include "Image.h"

#include "ts/tessa/file/InputFile.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/ivie/util/RenderUtil.h"

#include "ts/ivie/viewer/image/ImageManager.h"
#include "ts/ivie/viewer/image/ImageBackgroundLoaderFreeImage.h"

#include "ts/tessa/thread/ThreadScheduler.h"
#include "ts/ivie/viewer/image/ImageBackgroundLoaderWebm.h"

#include "ts/tessa/profiling/SimpleScopedZoneTimer.h"

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

// 	TS_SIMPLE_ZONE();

	ImageManager &im = TS_GET_GIGATON().getGigaton<ImageManager>();

	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());

	LoaderType type = sniffLoaderType();
	switch (type)
	{
		case LoaderFreeImage:
		{
			displayShader = im.loadDisplayShader(ImageManager::DisplayShader_FreeImage);

			loaderState = Loading;
			backgroundLoader.reset(new ImageBackgroundLoaderFreeImage(this, filepath));
		}
		break;

		case LoaderWebm:
		{
			displayShader = im.loadDisplayShader(ImageManager::DisplayShader_Webm);

			loaderState = Loading;
			backgroundLoader.reset(new ImageBackgroundLoaderWebm(this, filepath));
		}
		break;

		case LoaderUnknown:
		{
			loaderState = Error;
			TS_WLOG_ERROR("Unable to load file, unknown or unsupported type. File: %s", filepath);
			
			errorText = "Invalid image file or the format is unsupported.";

			return false;
		}
		break;
	}

	currentLoaderType = type;

	TS_ASSERT(backgroundLoader);
	backgroundLoader->start(suspendAfterBufferFull);

	return true;
}

void Image::unload()
{
	if (loaderState == Unloaded)
		return;

	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());

// 	TS_SIMPLE_ZONE();

	loaderState = Unloading;

	if (backgroundLoader != nullptr)
		backgroundLoader->stop();

	backgroundLoader.reset();

	frameBuffer.clear();

	data = ImageData();
	currentFrameIndex = 0;
	loaderState = Unloaded;
}

void Image::restart(bool suspend)
{
	if (loaderState == Error || loaderState == Complete)
		return;

	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());

	TS_ASSERT(backgroundLoader);

	if (currentFrameIndex > 0)
	{
		if (backgroundLoader->restart(true))
		{
			currentFrameIndex = 0;
			frameBuffer.clear();
		}
	}
// 	else if (loaderState == Loading)
// 	{
// 		backgroundLoader->suspend();
// 	}
	
}

void Image::suspendLoader()
{
	if (loaderState == Unloaded || loaderState == Error)
		return;

	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());

	TS_ASSERT(backgroundLoader);
	backgroundLoader->suspend();
}

void Image::setActive(bool activeParam)
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	active = activeParam;
	if (active && backgroundLoader)
	{
		backgroundLoader->cancelPendingSuspension();
	}
}

bool Image::isUnloaded() const
{
	return loaderState == Unloaded;
}

bool Image::isUnloading() const
{
	return loaderState == Unloading;
}

bool Image::isSuspended() const
{
	return loaderState == Suspended;// && backgroundLoader && backgroundLoader->isSuspended();
}

void Image::resumeLoading()
{
	TS_ASSERT(loaderState == Suspended);
	if (loaderState != Suspended)
		return;

	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	TS_ASSERT(backgroundLoader);
	backgroundLoader->resume();
}

const math::VC2U Image::getSize() const
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
// 	TS_ASSERT(data.size.x > 0 && data.size.y > 0 && "Image size is not set.");
	return data.size;
}

const FrameStorage *Image::getCurrentFrameStorage() const
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	if (!frameBuffer.isEmpty())
		return &frameBuffer.getReadPtr();
	return nullptr;
}

SharedPointer<sf::Shader> Image::getDisplayShader(const math::VC2 &apparentSize, bool useAlphaChecker)
{
	if (displayShader)
	{
		switch (currentLoaderType)
		{
			case LoaderFreeImage:
			{
				displayShader->setUniform("u_textureApparentSize", apparentSize);
				displayShader->setUniform("u_useAlphaChecker", true);
			}
			break;
			case LoaderWebm:
			{
// 				displayShader->setUniform("u_textureApparentSize", apparentSize);
// 				displayShader->setUniform("u_useAlphaChecker", true);
			}
			break;
			default: TS_ASSERT(!"Unknown type"); break;
		}
	}

	return displayShader;
}

SizeType Image::getCurrentFrameIndex() const
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	return currentFrameIndex;
}

SizeType Image::getNumFramesTotal() const
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	return data.numFramesTotal;
}

SizeType Image::getNumFramesBuffered() const
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	return (SizeType)frameBuffer.getBufferedAmount();
}

bool Image::getIsAnimated() const
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	return data.numFramesTotal > 1;
}

bool Image::getHasAlpha() const
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	return data.hasAlpha;
}

bool Image::advanceToNextFrame()
{
	static SizeType lastFrameIndex = ~0U;

	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());

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
	if (loaderState == Unloading)
		return false;

	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	return loaderState != Error && !frameBuffer.isEmpty() && displayShader != nullptr;
}

bool Image::hasError() const
{
	return loaderState == Error;
}

const std::string &Image::getErrorText() const
{
	return errorText;
}

bool Image::hasThumbnail() const
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	return thumbnail != nullptr;
}

SharedPointer<sf::Texture> Image::getThumbnail() const
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	return thumbnail;
}

void Image::setImageData(const ImageData &dataParam)
{
	if (loaderState == Unloading)
		return;

	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	data = dataParam;
}

Image::LoaderType Image::sniffLoaderType()
{
	// Test FreeImage formats
	if (ImageBackgroundLoaderFreeImage::isValidFreeImageFile(filepath))
		return LoaderFreeImage;

	// Test Nestegg format (webm)
	if (ImageBackgroundLoaderWebm::isValidWebmFile(filepath))
		return LoaderWebm;

	return LoaderUnknown;
}

Image::ImageLoaderState Image::getState() const
{
	return loaderState;
}

const std::wstring &Image::getFilepath() const
{
	// No mutexing because value doesn't ever change after creation, reads should always be ok
	return filepath;
}

std::wstring Image::getStats() const
{
	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	std::wstring str = TS_WFMT("%s (%u / %u [%u buffered]) Image: %s Loader: %s",
		file::getBasename(filepath),
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
	loaderState = state;
}

bool Image::getIsBufferFull() const
{
	if (loaderState == Unloading)
		return true;

	return frameBuffer.isFull();
}

FrameStorage *Image::getNextBuffer()
{
	if (loaderState == Unloading)
		return nullptr;

	return &frameBuffer.getWritePtr();
}

void Image::swapBuffer()
{
	if (loaderState == Unloading)
		return;

	frameBuffer.incrementWrite();

	if (makingThumbnail == false)
	{
		const FrameStorage &storage = frameBuffer.getReadPtr();
		TS_ASSERT(storage.texture != nullptr);

		thread::ThreadScheduler &ts = TS_GET_GIGATON().getGigaton<thread::ThreadScheduler>();
		ts.scheduleOnce(
			thread::Priority_Normal, TimeSpan::zero,
			&ThisClass::makeThumbnail, this,
			storage.texture, 300);

		makingThumbnail = true;
	}
}

void Image::finalizeBuffer()
{
	if (loaderState == Unloading)
		return;

// 	MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
	frameBuffer.removeReadConstraint();
}

bool Image::makeThumbnail(SharedPointer<sf::Texture> frameTexture, SizeType maxSize)
{
	TS_ASSERT(maxSize > 0);
	TS_ASSERT(frameTexture != nullptr);
	if (frameTexture == nullptr)
		return false;

	math::VC2U textureSize = frameTexture->getSize();
	TS_ASSERT(textureSize.x != 0 && textureSize.y != 0);
	if (textureSize.x == 0 || textureSize.y == 0)
		return false;

	math::VC2U scaledSize;
	if (textureSize.x >= textureSize.y)
	{
		float factor = maxSize / (float)textureSize.x;
		scaledSize.x = maxSize;
		scaledSize.y = (uint32)(textureSize.y * factor);
	}
	else
	{
		float factor = maxSize / (float)textureSize.y;
		scaledSize.y = maxSize;
		scaledSize.x = (uint32)(textureSize.x * factor);
	}

	sf::RenderTexture rt;
	if (!rt.create(scaledSize.x, scaledSize.y))
		return false;

	sf::RenderStates states;
	states.texture = frameTexture.get();

	states.shader = getDisplayShader(static_cast<math::VC2>(scaledSize), true).get();

	rt.clear(sf::Color::White);
	rt.draw(
		util::makeQuadVertexArrayScaled(scaledSize.x, scaledSize.y, textureSize.x, textureSize.y),
		states);
	rt.display();

	sf::Texture *thumbnailTexture = new sf::Texture(rt.getTexture());
	TS_ASSERT(thumbnailTexture != nullptr);
	if (thumbnailTexture != nullptr)
	{
		thumbnailTexture->setSmooth(true);
		thumbnailTexture->generateMipmap();

		MutexGuard lock(mutex, MUTEXGUARD_DEBUGINFO());
		thumbnail.reset(thumbnailTexture);
	}

	return thumbnail != nullptr;
}

TS_END_PACKAGE2()
