#pragma once

#include "ts/ivie/viewer/image/AbstractImageBackgroundLoader.h"

#include "ts/tessa/file/InputFile.h"

#include "SFML/Graphics.hpp"
#include "FreeImage.h"

TS_DECLARE2(app, viewer, Image);

TS_PACKAGE2(app, viewer)

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

	virtual bool restartImpl() override;

	virtual bool loadNextFrame(FrameStorage &bufferStorage) override;
	virtual bool wasLoadingCompleted() const override;

private:
	bool prepareForLoading();
	void cleanup();

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
	Image::ImageData imageData;

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

	std::string disposalToString(DisposalMethod dm)
	{
		switch (dm)
		{
			case DisposalMethod_Unspecified: return "Unspecified";
			case DisposalMethod_Leave: return "Leave";
			case DisposalMethod_Background: return "Background";
			case DisposalMethod_Previous: return "Previous";
		}
		return "Not set";
	}

};

TS_END_PACKAGE2()
