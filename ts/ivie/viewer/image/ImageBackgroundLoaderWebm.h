#pragma once

#include "ts/ivie/viewer/image/AbstractImageBackgroundLoader.h"

#include "ts/tessa/file/InputFile.h"

#include <deque>

struct nestegg;

struct vpx_codec_iface;
typedef const struct vpx_codec_iface vpx_codec_iface_t;

struct vpx_codec_ctx;
typedef vpx_codec_ctx vpx_codec_ctx_t;

TS_DECLARE2(app, viewer, Image);

TS_PACKAGE2(app, viewer)

class ImageBackgroundLoaderWebm : public AbstractImageBackgroundLoader
{
	typedef ImageBackgroundLoaderWebm ThisClass;
	typedef AbstractImageBackgroundLoader BaseClass;

public:
	ImageBackgroundLoaderWebm(Image *ownerImage, const String &filepath);
	virtual ~ImageBackgroundLoaderWebm();

	virtual bool isLoadingComplete() const override;

	static bool isValidWebmFile(const String &filepath);

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
	void cleanup();

	bool processNextFrame(FrameStorage &bufferStorage);

	bool loaderIsPrepared = false;
	bool loaderIsComplete = false;

	file::InputFile fileHandle;

	struct DecoderState
	{
		nestegg *context = nullptr;
		vpx_codec_iface_t *interface = nullptr;
		vpx_codec_ctx_t *codec = nullptr;

		SizeType trackIndex = 0;
	};
	DecoderState state;

	uint32 numFrames = 0;
	uint32 numTotalFrames = 0;

	TimeSpan frameTime;
	TimeSpan totalDuration;

	struct BufferedFrame
	{
		SharedPointer<sf::Texture> texture;
		TimeSpan frameTime;
	};
	std::deque<BufferedFrame> bufferedFrames;

	math::VC2U imageSize;

	bool imageDataUpdated = false;
	Image::ImageData imageData;

};

TS_END_PACKAGE2()
