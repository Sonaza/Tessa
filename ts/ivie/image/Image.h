#pragma once

#include "ts/container/RingBuffer.h"

TS_DECLARE2(app, image, AbstractImageBackgroundLoader);

TS_PACKAGE2(app, image)

struct FrameStorage
{
	SharedPointer<sf::Texture> texture;
	TimeSpan frameTime;
};

struct ImageData
{
	math::VC2U size;
	bool hasAlpha = false;
	uint32_t numFramesTotal = 0;
	bool canBeRotated = false;
};

struct DisplayShaderParams
{
	math::VC2 viewSize;
	float scale;
	math::VC2 offset;
};

class Image
{
	typedef Image ThisClass;

	friend class AbstractImageBackgroundLoader;
	friend class ImageBackgroundLoaderFreeImage;
	friend class ImageBackgroundLoaderWebm;

public:
	Image(const String &filepath);
	~Image();

	bool startLoading(bool suspendAfterBufferFull);
	void unload();

	bool reload();

	void restart(bool suspend);
	void suspendLoader();

	void setActive(bool active);

	bool isUnloaded() const;
	bool isUnloading() const;
	bool isSuspended() const;
	void resumeLoading();

	bool getIsAnimated() const;
	float getAnimationProgress(TimeSpan frametime = TimeSpan::zero) const;

	bool getImageData(ImageData &outData) const;
	const math::VC2U getSize() const;

	FrameStorage *getCurrentFrameStorage();

	sf::Shader *getDisplayShader(const DisplayShaderParams *params);

	SizeType getCurrentFrameIndex() const;
	SizeType getNumFramesTotal() const;
	SizeType getNumFramesBuffered() const;

	bool advanceToNextFrame();

	bool isDisplayable() const;

	bool hasError() const;
	const String &getErrorText() const;

	bool hasThumbnail() const;
	SharedPointer<sf::Texture> getThumbnail() const;

	enum RotateDirection
	{
		Rotation_Clockwise,
		Rotation_CounterClockwise,
	};
	bool rotate(RotateDirection direction, bool saveToDisk = true);
	bool canImageBeRotated() const;

	enum ImageLoaderState
	{
		Unloaded,
		Loading,
		Complete,
		Suspended,
		Unloading,
		Error,
	};
	ImageLoaderState getState() const;

	String getStateString(ImageLoaderState state) const
	{
		switch (state)
		{
			case Unloaded:  return L"Unloaded";
			case Loading:   return L"Loading";
			case Complete:  return L"Complete";
			case Suspended: return L"Suspended";
			case Unloading: return L"Unloading";
			case Error:     return L"Error";
		}
		TS_ASSERT(!"All states not handled.");
		return L"Unknown";
	}

	const String &getFilepath() const;

	String getStats() const;

private:
	bool getIsBufferFull() const;
	FrameStorage *getNextBuffer();
	void swapBuffer();
	void finalizeBuffer();

	bool makeThumbnail(SharedPointer<sf::Texture> frameTexture, SizeType maxSize);

	String filepath;
	bool active = false;

	void setImageData(const ImageData &imageData);
	ImageData imageData;
	bool imageDataIsSet = false;

	enum LoaderType
	{
		LoaderUnknown,
		LoaderFreeImage,
		LoaderWebm,

		LoaderErrorFileMissing,
	};
	LoaderType sniffLoaderType();
	LoaderType currentLoaderType = LoaderUnknown;

	void setState(ImageLoaderState state);
	std::atomic<ImageLoaderState> loaderState = Unloaded;

	String errorText;

	SizeType currentFrameIndex = 0;
	TimeSpan currentFrameTime;

	SizeType displayableBufferThreshold = 1;
	bool displayableThresholdReached = false;

	static const BigSizeType MaxFrameBufferCapacity = 20;
	typedef util::RingBuffer<FrameStorage, MaxFrameBufferCapacity> FrameRingBuffer;
	FrameRingBuffer frameBuffer;

	bool makingThumbnail = false;
	SharedPointer<sf::Texture> thumbnail;
	SharedPointer<resource::ShaderResource> displayShader;

	ScopedPointer<AbstractImageBackgroundLoader> backgroundLoader;
	
	mutable Mutex mutex;
};

TS_END_PACKAGE2()
