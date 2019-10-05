#pragma once

#include "ts/ivie/image/AbstractImageBackgroundLoader.h"

#include "ts/file/InputFile.h"

#include "SFML/Graphics.hpp"
#include "FreeImage.h"

TS_DECLARE2(app, image, Image);

TS_PACKAGE2(app, image)

class ImageBackgroundLoaderFreeImage : public AbstractImageBackgroundLoader
{
	typedef ImageBackgroundLoaderFreeImage ThisClass;
	typedef AbstractImageBackgroundLoader BaseClass;

public:
	ImageBackgroundLoaderFreeImage(Image *ownerImage, const String &filepath);
	virtual ~ImageBackgroundLoaderFreeImage();

	virtual bool isLoadingComplete() const override;

	static bool isValidFreeImageFile(const String &filepath);

protected:
	virtual bool initialize() override;
	virtual void deinitialize() override;

	virtual void onResume() override;
	virtual void onSuspend() override;

	virtual bool restartImpl(bool *shouldRestart) override;

	virtual bool loadNextFrame(FrameStorage &bufferStorage) override;
	virtual bool wasLoadingCompleted() const override;

private:
	bool prepareForLoading();
	void cleanup(bool soft = false);

	bool processNextStill(FrameStorage &bufferStorage);
	bool processNextMultiBitmap(FrameStorage &bufferStorage);

	bool loaderIsPrepared = false;
	bool loaderIsComplete = false;

	struct FreeImageState
	{
		FREE_IMAGE_FORMAT format = FIF_UNKNOWN;

		std::vector<BYTE> memoryBuffer;
		FIMEMORY *memory = nullptr;

		union
		{
			FIBITMAP *bitmap;
			FIMULTIBITMAP *multibitmap;
		};
	};
	FreeImageState state;

	sf::VertexArray imageVertexArray;
	SharedPointer<sf::Texture> previousFrame;
	UniquePointer<sf::RenderTexture> stackingRenderTexture;
	int32 stackingRenderTextureThreadId = -1;

	// Storage for render texture before suspension since its GL context requires that the 
	// creation and deletion happen in the same thread. Gotta deinit before suspending.
	SharedPointer<sf::Texture> savedRenderTexture;

	math::VC2U imageSize;
	SizeType currentPage = 0;
	SizeType numPagesTotal = 0;

	bool imageDataUpdated = false;
	ImageData imageData;

	bool multibitmapInitialized = false;

	enum FormatType
	{
		Unspecified,
		StillImageFormat,
		MultiBitmapFormat,
	};
	FormatType loaderFormat = Unspecified;

	enum DisposalMethod : uint8_t
	{
		DisposalMethod_Unspecified = 0,
		DisposalMethod_Leave = 1,
		DisposalMethod_Background = 2,
		DisposalMethod_Previous = 3,

		DisposalMethod_NotSet = 255,
	};
	DisposalMethod lastDisposalMethod = DisposalMethod_NotSet;
};

TS_END_PACKAGE2()
