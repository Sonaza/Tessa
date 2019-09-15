#include "Precompiled.h"
#include "Image.h"

#include "ts/tessa/file/InputFile.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/ivie/util/RenderUtil.h"

#include "ts/ivie/viewer/ViewerManager.h"
#include "ts/ivie/image/ImageBackgroundLoaderFreeImage.h"

#include "ts/tessa/thread/ThreadScheduler.h"
#include "ts/ivie/image/ImageBackgroundLoaderWebm.h"

#include "ts/tessa/profiling/ZoneProfiler.h"

TS_PACKAGE2(app, image)

using viewer::ViewerManager;

Image::Image(const String &filepath)
	: filepath(filepath)
{
	TS_ASSERT(!filepath.isEmpty());
}

Image::~Image()
{
	backgroundLoader.reset();
}

bool Image::reload()
{
	unload();
	return startLoading(false);
}

bool Image::startLoading(bool suspendAfterBufferFull)
{
	TS_ZONE();

	TS_ASSERT(loaderState == Unloaded);
	if (loaderState != Unloaded)
		return false;

// 	TS_ZONE();

	ViewerManager &vm = TS_GET_GIGATON().getGigaton<ViewerManager>();

	MutexGuard lock(mutex);

	errorText = "Unknown error.";

	LoaderType type = sniffLoaderType();
	switch (type)
	{
		case LoaderFreeImage:
		{
			displayShader = vm.loadDisplayShader(ViewerManager::DisplayShader_FreeImage);

			loaderState = Loading;
			backgroundLoader.reset(new ImageBackgroundLoaderFreeImage(this, filepath));
		}
		break;

		case LoaderWebm:
		{
			displayShader = vm.loadDisplayShader(ViewerManager::DisplayShader_Webm);

			loaderState = Loading;
			backgroundLoader.reset(new ImageBackgroundLoaderWebm(this, filepath));
		}
		break;

		case LoaderUnknown:
		{
			loaderState = Error;
			TS_WLOG_ERROR("Unable to load file, unknown or unsupported type. File: %s", filepath);
			
			errorText = "File format is unsupported or the file is corrupted.";

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
	TS_ZONE();

	if (loaderState == Unloaded)
		return;

	MutexGuard lock(mutex);

// 	TS_ZONE();

	loaderState = Unloading;

	if (backgroundLoader != nullptr)
		backgroundLoader->stop();

	backgroundLoader.reset();

	frameBuffer.clear();

	errorText.clear();

	data = ImageData();
	currentFrameIndex = 0;
	loaderState = Unloaded;
}

void Image::restart(bool suspend)
{
	if (loaderState == Error || loaderState == Complete)
		return;

	MutexGuard lock(mutex);

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
	MutexGuard lock(mutex);
	if (loaderState == Unloaded || loaderState == Error)
		return;

	TS_ASSERT(backgroundLoader);
	backgroundLoader->suspend();
}

void Image::setActive(bool activeParam)
{
	MutexGuard lock(mutex);
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

	MutexGuard lock(mutex);
	TS_ASSERT(backgroundLoader);
	backgroundLoader->resume();
}

const math::VC2U Image::getSize() const
{
	TS_ZONE();

	MutexGuard lock(mutex);
// 	TS_ASSERT(data.size.x > 0 && data.size.y > 0 && "Image size is not set.");
	return data.size;
}

FrameStorage *Image::getCurrentFrameStorage()
{
	TS_ZONE();

	MutexGuard lock(mutex);
	if (!frameBuffer.isEmpty())
		return &frameBuffer.getReadPtr();
	return nullptr;
}

sf::Shader *Image::getDisplayShader(const float apparentScale)
{
	TS_ZONE();

	sf::Shader *shader = nullptr;

	if (displayShader)
	{
		TS_ZONE_NAMED("Set Uniforms");

		shader = displayShader->getResource().get();

		switch (currentLoaderType)
		{
			case LoaderFreeImage:
			{
				sf::Glsl::Vec2 arr[2];
				arr[0] = (math::VC2)data.size * apparentScale;
				arr[1] = math::VC2(apparentScale, apparentScale);
				shader->setUniformArray("u_params", arr, 2);

// 				shader->setUniform("u_textureApparentSize", apparentSize);
// 				shader->setUniform("u_apparentScale", apparentScale);
			}
			break;

			case LoaderWebm:
			{

			}
			break;

			default: TS_ASSERT(!"Unknown type"); break;
		}
	}

	return shader;
}

SizeType Image::getCurrentFrameIndex() const
{
	TS_ZONE();

	MutexGuard lock(mutex);
	return currentFrameIndex;
}

SizeType Image::getNumFramesTotal() const
{
	TS_ZONE();

	MutexGuard lock(mutex);
	return data.numFramesTotal;
}

SizeType Image::getNumFramesBuffered() const
{
	TS_ZONE();

	MutexGuard lock(mutex);
	return (SizeType)frameBuffer.getBufferedAmount();
}

bool Image::getIsAnimated() const
{
	TS_ZONE();

	MutexGuard lock(mutex);
	return data.numFramesTotal > 1;
}

float Image::getAnimationProgress(TimeSpan elapsedFrameTime) const
{
	if (data.numFramesTotal == 0)
		return 0.f;

	float progress = (currentFrameIndex / (float)data.numFramesTotal) +
		(elapsedFrameTime.getMilliseconds() / (float)currentFrameTime.getMilliseconds()) * (1.f / (float)data.numFramesTotal);

	return progress;
}

bool Image::getHasAlpha() const
{
	TS_ZONE();

	MutexGuard lock(mutex);
	return data.hasAlpha;
}

bool Image::advanceToNextFrame()
{
	TS_ZONE();

	static SizeType lastFrameIndex = ~0U;

	MutexGuard lock(mutex);

	backgroundLoader->requestNextFrame();

	if (loaderState == Complete || frameBuffer.canIncrementRead())
	{
		frameBuffer.incrementRead();
		currentFrameIndex = (currentFrameIndex + 1) % data.numFramesTotal;
		currentFrameTime = frameBuffer.getReadPtr().frameTime;
		return true;
	}
	return false;
}

bool Image::isDisplayable() const
{
	TS_ZONE();

	if (loaderState == Unloading)
		return false;

	MutexGuard lock(mutex);
	return loaderState != Error && !frameBuffer.isEmpty() && displayShader != nullptr;
}

bool Image::hasError() const
{
	return loaderState == Error;
}

const String &Image::getErrorText() const
{
	return errorText;
}

bool Image::hasThumbnail() const
{
	TS_ZONE();

	MutexGuard lock(mutex);
	return thumbnail != nullptr;
}

SharedPointer<sf::Texture> Image::getThumbnail() const
{
	TS_ZONE();

	MutexGuard lock(mutex);
	return thumbnail;
}

void Image::setImageData(const ImageData &dataParam)
{
	TS_ZONE();

	if (loaderState == Unloading)
		return;

	MutexGuard lock(mutex);
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

const String &Image::getFilepath() const
{
	TS_ZONE();

	// No mutexing because value doesn't ever change after creation, reads should always be ok
	return filepath;
}

String Image::getStats() const
{
	MutexGuard lock(mutex);
	String str = TS_WFMT("%s (%u / %u [%u buffered]) Image: %s Loader: %s",
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
	TS_ZONE();

	if (loaderState == Unloading)
		return true;

	return frameBuffer.isFull();
}

FrameStorage *Image::getNextBuffer()
{
	TS_ZONE();

	if (loaderState == Unloading)
		return nullptr;

	return &frameBuffer.getWritePtr();
}

void Image::swapBuffer()
{
	TS_ZONE();

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

// 	MutexGuard lock(mutex);
	frameBuffer.removeReadConstraint();
}

bool Image::makeThumbnail(SharedPointer<sf::Texture> frameTexture, SizeType maxSize)
{
	TS_ZONE();
	
	TS_ASSERT(maxSize > 0);
	TS_ASSERT(frameTexture != nullptr);
	if (frameTexture == nullptr)
		return false;

	math::VC2U textureSize = frameTexture->getSize();
	TS_ASSERT(textureSize.x != 0 && textureSize.y != 0);
	if (textureSize.x == 0 || textureSize.y == 0)
		return false;

	float scaleFactor;

	math::VC2U scaledSize;
	if (textureSize.x >= textureSize.y)
	{
		scaleFactor = maxSize / (float)textureSize.x;
		scaledSize.x = maxSize;
		scaledSize.y = (uint32)(textureSize.y * scaleFactor);
	}
	else
	{
		scaleFactor = maxSize / (float)textureSize.y;
		scaledSize.y = maxSize;
		scaledSize.x = (uint32)(textureSize.x * scaleFactor);
	}

	sf::RenderTexture rt;
	{
		TS_ZONE_NAMED("RenderTexture thumbnail");

		if (!rt.create(scaledSize.x, scaledSize.y))
			return false;

		sf::RenderStates states;
		states.texture = frameTexture.get();

		states.shader = getDisplayShader(scaleFactor);

		rt.clear(sf::Color::White);
		rt.draw(
			util::makeQuadVertexArrayScaled(scaledSize.x, scaledSize.y, textureSize.x, textureSize.y),
			states);
		rt.display();
	}

	sf::Texture *thumbnailTexture = new sf::Texture(rt.getTexture());
	TS_ASSERT(thumbnailTexture != nullptr);
	if (thumbnailTexture != nullptr)
	{
		thumbnailTexture->setSmooth(true);
		thumbnailTexture->generateMipmap();

		MutexGuard lock(mutex);
		thumbnail.reset(thumbnailTexture);
	}

	return thumbnail != nullptr;
}

TS_END_PACKAGE2()