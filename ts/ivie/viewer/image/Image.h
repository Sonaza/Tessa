#pragma once

#include "ts/ivie/util/RingBuffer.h"

#include <exception>

TS_DECLARE2(app, viewer, AbstractImageBackgroundLoader);

TS_PACKAGE2(app, viewer)

struct FrameStorage
{
	SharedPointer<sf::Texture> texture;
	TimeSpan frameTime;
};

class ImageUnloadingException : public std::exception
{
public:
	ImageUnloadingException() = default;
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

	void setActive(bool active);

	bool isUnloaded() const;
	bool isUnloading() const;
	bool isSuspended() const;
	void resumeLoading();

	bool getIsAnimated() const;
	bool getHasAlpha() const;
	const math::VC2U getSize() const;

	const FrameStorage *getCurrentFrameStorage() const;
	SharedPointer<sf::Shader> getDisplayShader() const;

	SizeType getCurrentFrameIndex() const;
	SizeType getNumFramesTotal() const;
	SizeType getNumFramesBuffered() const;

	bool advanceToNextFrame();

	bool isDisplayable() const;
	bool hasError() const;

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

	std::wstring getStateString(ImageLoaderState state) const
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

	const std::wstring &getFilepath() const;

	std::wstring getStats() const;

private:
	bool getIsBufferFull() const;
	FrameStorage &getNextBuffer();
	void swapBuffer();
	void finalizeBuffer();

	bool makeThumbnail(const FrameStorage &bufferStorage, SizeType maxSize);

	std::wstring filepath;
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

	void setState(ImageLoaderState state);
	std::atomic<ImageLoaderState> loaderState = Unloaded;

	SizeType currentFrameIndex = 0;

	static const BigSizeType MaxFrameBufferCapacity = 30;
	typedef util::RingBuffer<FrameStorage, MaxFrameBufferCapacity> FrameRingBuffer;
	FrameRingBuffer frameBuffer;
	SharedPointer<sf::Texture> thumbnail;
	SharedPointer<sf::Shader> displayShader;

	ScopedPointer<AbstractImageBackgroundLoader> backgroundLoader;
	
	mutable std::mutex mutex;
};

TS_END_PACKAGE2()
