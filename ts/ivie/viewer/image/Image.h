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
	friend class AbstractImageBackgroundLoader;
	friend class ImageBackgroundLoaderFreeImage;
	friend class ImageBackgroundLoaderWebm;

public:
	Image(const std::wstring &filepath);
	~Image();

	bool startLoading(bool suspendAfterBufferFull);
	void unload();

	void restart(bool suspend);

	bool isUnloaded() const;
	bool isSuspended() const;
	void resumeLoading();

	bool getIsAnimated() const;
	bool getHasAlpha() const;
	const math::VC2U getSize() const;

	const FrameStorage *getCurrentFrameStorage() const;

	SizeType getCurrentFrameIndex() const;
	SizeType getNumFramesTotal() const;
	SizeType getNumFramesBuffered() const;

	bool advanceToNextFrame();

	bool isDisplayable() const;
	bool hasError() const;

	enum ImageLoaderState
	{
		Unloaded,
		Loading,
		Complete,
		Suspended,
		Error,
	};
	ImageLoaderState getState() const;

	std::wstring getStateString(ImageLoaderState state) const
	{
		switch (state)
		{
			case Unloaded: return L"Unloaded";
			case Loading: return L"Loading";
			case Complete: return L"Complete";
			case Suspended: return L"Suspended";
			case Error: return L"Error";
		}
		return L"Unknown";
	}

	const std::wstring &getFilepath() const;

	std::wstring getStats() const;

private:
	std::wstring filepath;

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

	void setState(ImageLoaderState state);
	ImageLoaderState loaderState = Unloaded;

	SizeType currentFrameIndex = 0;

	static const BigSizeType MaxFrameBufferCapacity = 30;
	typedef util::RingBuffer<FrameStorage, MaxFrameBufferCapacity> FrameRingBuffer;
	FrameRingBuffer frameBuffer;

	ScopedPointer<AbstractImageBackgroundLoader> backgroundLoader;
	
	mutable std::mutex mutex;
};

TS_END_PACKAGE2()
