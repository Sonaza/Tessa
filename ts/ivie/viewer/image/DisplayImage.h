#pragma once

#include "ts/ivie/util/RingBuffer.h"

TS_DECLARE2(app, viewer, AbstractImageBackgroundLoader);

TS_PACKAGE2(app, viewer)

struct FrameStorage
{
	SharedPointer<sf::Texture> texture;
	TimeSpan frameTime;
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

	void restart(bool suspend);
	void suspendLoader();

	void setActive(bool active);

	bool isUnloaded() const;
	bool isUnloading() const;
	bool isSuspended() const;
	void resumeLoading();

	bool getIsAnimated() const;
	bool getHasAlpha() const;
	const math::VC2U getSize() const;

	const FrameStorage *getCurrentFrameStorage() const;

	SharedPointer<sf::Shader> getDisplayShader(const math::VC2 &apparentSize, bool useAlphaChecker);

	SizeType getCurrentFrameIndex() const;
	SizeType getNumFramesTotal() const;
	SizeType getNumFramesBuffered() const;

	bool advanceToNextFrame();

	bool isDisplayable() const;

	bool hasError() const;
	const String &getErrorText() const;

	bool hasThumbnail() const;
	SharedPointer<sf::Texture> getThumbnail() const;

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

	struct ImageData
	{
		math::VC2U size;
		bool hasAlpha = false;
		SizeType numFramesTotal = 0;
	};
	void setImageData(const ImageData &imageData);
	ImageData data;

	enum LoaderType
	{
		LoaderUnknown,
		LoaderFreeImage,
		LoaderWebm,
	};
	LoaderType sniffLoaderType();
	LoaderType currentLoaderType = LoaderUnknown;

	void setState(ImageLoaderState state);
	std::atomic<ImageLoaderState> loaderState = Unloaded;

	String errorText;

	SizeType currentFrameIndex = 0;

	static const BigSizeType MaxFrameBufferCapacity = 20;
	typedef util::RingBuffer<FrameStorage, MaxFrameBufferCapacity> FrameRingBuffer;
	FrameRingBuffer frameBuffer;

	bool makingThumbnail = false;
	SharedPointer<sf::Texture> thumbnail;
	SharedPointer<sf::Shader> displayShader;

	ScopedPointer<AbstractImageBackgroundLoader> backgroundLoader;
	
	mutable Mutex mutex;
};

TS_END_PACKAGE2()