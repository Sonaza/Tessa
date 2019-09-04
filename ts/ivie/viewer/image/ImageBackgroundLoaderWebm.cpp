#include "Precompiled.h"
#include "ImageBackgroundLoaderWebm.h"

#include "ts/tessa/thread/Thread.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/ivie/viewer/image/Image.h"
#include "ts/ivie/util/RenderUtil.h"

#include "ts/tessa/profiling/ScopedZoneTimer.h"

#define HAVE_STDINT_H 1
#include "nestegg/nestegg.h"
#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"

#pragma warning( disable: 4505 ) // Unreferenced local function has been removed

TS_PACKAGE2(app, viewer)

namespace
{

int io_read(void *buffer, size_t size, void *userdata)
{
	file::InputFile &handle = *(file::InputFile*)userdata;
	if (handle.isEOF())
		return 0;

	PosType bytesRead = (int)handle.read(reinterpret_cast<char*>(buffer), size);
	return bytesRead > 0 ? 1 : (handle.isEOF() ? 0 : -1); // 1 on success, 0 on eof, -1 on failure
}

int io_seek(int64_t position, int whence, void *userdata)
{
	file::InputFile &handle = *(file::InputFile*)userdata;

	file::InputFile::SeekOrigin origin;
	switch (whence)
	{
		case NESTEGG_SEEK_SET: origin = file::InputFile::SeekFromBeginning; break;
		case NESTEGG_SEEK_CUR: origin = file::InputFile::SeekFromCurrent; break;
		case NESTEGG_SEEK_END: origin = file::InputFile::SeekFromEnd; break;
		default: TS_ASSERT(!"Unexpected seek origin."); return -1;
	}

	handle.seek(position, origin);
	if (handle.isBad())
	{
		TS_ASSERT(!"The baddening");
		return -1;
	}
	return 0;
}

int64_t io_tell(void* userdata)
{
	file::InputFile &handle = *(file::InputFile*)userdata;
	return handle.tell();
}

void nestegg_log_callback(nestegg *context, unsigned int severity, char const* format, ...)
{
	va_list args;
	va_start(args, format);

	const SizeType bufferSize = 1024;
	char buffer[1024] = { 0 };

#if TS_PLATFORM == TS_WINDOWS
	vsprintf_s(buffer, bufferSize, format, args);
#else
	vsprintf(buffer, format, args);
#endif
	va_end(args);

	switch (severity)
	{
		case NESTEGG_LOG_CRITICAL:
			TS_PRINTF("[Nestegg CRITICAL] : %s\n", buffer);
		break;
		case NESTEGG_LOG_ERROR:
			TS_PRINTF("[Nestegg ERROR]    : %s\n", buffer);
		break;
		case NESTEGG_LOG_WARNING:
			TS_PRINTF("[Nestegg WARNING]  : %s\n", buffer);
			break;
		case NESTEGG_LOG_INFO:
			TS_PRINTF("[Nestegg INFO]     : %s\n", buffer);
		break;
		case NESTEGG_LOG_DEBUG:
// 			TS_PRINTF("[Nestegg DEBUG]    : %s\n", buffer);
		break;

		default: /* bop */ break;
	}
}

}

ImageBackgroundLoaderWebm::ImageBackgroundLoaderWebm(Image *ownerImage, const String &filepath)
	: AbstractImageBackgroundLoader(ownerImage, filepath)
{
	TS_ZONE();
}

ImageBackgroundLoaderWebm::~ImageBackgroundLoaderWebm()
{
	TS_ASSERTF(loaderIsPrepared == false, "Cleanup is incomplete. Task ID %u", taskId);
}

bool ImageBackgroundLoaderWebm::initialize()
{
	return true;
}

void ImageBackgroundLoaderWebm::deinitialize()
{
	cleanup();
}

void ImageBackgroundLoaderWebm::onResume()
{
	
}

void ImageBackgroundLoaderWebm::onSuspend()
{
	
}

bool ImageBackgroundLoaderWebm::prepareForLoading()
{
	TS_ZONE();

	TS_ASSERT(loaderIsPrepared == false && "Loader is already prepared.");

	if (!fileHandle.open(filepath, file::InputFileMode_ReadBinary))
	{
		TS_WLOG_ERROR("Failed to open file. File: %s\n", filepath);
		errorText = "Failed to open file. File doesn't exist?";
		return false;
	}

	nestegg_io ne_io;
	ne_io.read = &io_read;
	ne_io.seek = &io_seek;
	ne_io.tell = &io_tell;
	ne_io.userdata = (void *)&fileHandle;

	

	if (nestegg_init(&state.context, ne_io, &nestegg_log_callback, -1) == -1)
	{
		errorText = "Failed to initialize demuxer.";
		return false;
	}

	TS_ASSERT(state.context != nullptr);

	uint64 streamTotalDuration = 0;
	if (nestegg_duration(state.context, &streamTotalDuration) == -1)
	{
		errorText = "Failed to retrieve duration.";
		return false;
	}

	totalDuration = TimeSpan::fromNanoseconds(streamTotalDuration);

	TS_PRINTF("  Duration %llu milliseconds\n", totalDuration.getMilliseconds());

	SizeType numTracks = 0;
	if (nestegg_track_count(state.context, &numTracks) == -1)
	{
		errorText = "Failed to retrieve track count.";
		return false;
	}

	nestegg_video_params vparams;
	memset(&vparams, 0, sizeof(vparams));

	bool hasVideoTrack = false;

// 	vpx_codec_iface_t *interface = nullptr;
	for (SizeType track = 0; track < numTracks; ++track)
	{
		int32 trackType = nestegg_track_type(state.context, track);
		if (trackType == NESTEGG_TRACK_VIDEO)
		{
			state.trackIndex = track;

			int32 codec_id = nestegg_track_codec_id(state.context, track);
			TS_ASSERT(codec_id >= 0);

			switch (codec_id)
			{
				case NESTEGG_CODEC_VP8: state.interface = &vpx_codec_vp8_dx_algo; break;
				case NESTEGG_CODEC_VP9: state.interface = &vpx_codec_vp9_dx_algo; break;
				default:
					errorText = "Unsupported codec.";
					TS_ASSERTF(false, "Unsupported codec id: %d", codec_id);
				return false;
			}
			TS_ASSERT(state.interface != nullptr);

// 			TS_PRINTF("Video Track %u codec id: %d\n", track, codec_id);
// 			TS_PRINTF("Using codec interface %s\n", vpx_codec_iface_name(state.interface));

			if (nestegg_track_video_params(state.context, track, &vparams) == -1)
			{
				errorText = "Failed to retrieve video parameters.";
				return false;
			}

			uint64 defaultDuration = 0;
			if (nestegg_track_default_duration(state.context, track, &defaultDuration) == -1)
			{
// 				defaultDuration = 33000000;
				frameTime = 33_ms;
			}
			else
			{
				frameTime = TimeSpan::fromNanoseconds(defaultDuration);
			}
			
			numTotalFrames = (uint32)(totalDuration / frameTime);

// 			TS_PRINTF("Video has about %u frames\n", numTotalFrames);

// 			float framerate = 1.f / frameTime.getSecondsAsFloat();
// 			TS_PRINTF("  Frame time: %ums\n", frametime);
// 			TS_PRINTF("  FPS: %0.2f\n", framerate);
// 			TS_PRINTF("  Size: %u x %u\n", vparams.width, vparams.height);
// 			TS_PRINTF("  Display size: %u x %u\n", vparams.display_width, vparams.display_height);

			hasVideoTrack = true;
			break;
		}
	}

	if (!hasVideoTrack)
	{
		errorText = "File does not contain a video track.";
		return false;
	}

	imageSize = math::VC2U(
		vparams.display_width,
		vparams.display_height
	);

	imageData.hasAlpha = (vparams.alpha_mode == 1);
	imageData.size = imageSize;
	imageData.numFramesTotal = numTotalFrames;

	state.codec = new vpx_codec_ctx_t;
	if (state.codec == nullptr)
	{
		errorText = "Failed to initialize codec.";
		return false;
	}

// 	vpx_codec_dec_cfg_t cfg;
// 	memset(&cfg, 0, sizeof(cfg));
// 	cfg.threads = 4;

	int32 flags = 0;
	vpx_codec_err_t error = vpx_codec_dec_init(state.codec, state.interface, nullptr, flags);
	if (error != VPX_CODEC_OK)
	{
		errorText = "Failed to initialize codec.";
		TS_PRINTF("vpx_codec_dec_init failed. error: %s (%d)\n", vpx_codec_err_to_string(error), error);
		return false;
	}

	loaderIsPrepared = true;

	return true;
}

void ImageBackgroundLoaderWebm::cleanup()
{
	if (state.codec != nullptr)
	{
		vpx_codec_destroy(state.codec);
		delete state.codec;
		state.codec = nullptr;
	}

	if (state.context != nullptr)
	{
		nestegg_destroy(state.context);
		state.context = nullptr;
	}

	fileHandle.close();

	loaderIsPrepared = false;
	loaderIsComplete = false;

	TS_PRINTF("Cleanup complete.\n");
}

bool ImageBackgroundLoaderWebm::processNextFrame(FrameStorage &bufferStorage)
{
	int32 result = 0;

	enum ProcessingResult
	{
		Undefined,
		Skipping,
		Success,
		Error,
	};
	ProcessingResult processingResult = Undefined;

	struct NesteggPacketDeleter
	{
		void operator()(nestegg_packet *packet)
		{
			nestegg_free_packet(packet);
		}
	};

	while (true)
	{
		if (processingResult == Skipping)
			processingResult = Undefined;

		ScopedPointer<nestegg_packet, NesteggPacketDeleter> nesteggPacketDeleter;

		nestegg_packet *packet = nullptr;
		result = nestegg_read_packet(state.context, &packet);

		if (packet != nullptr)
			nesteggPacketDeleter.reset(packet);

		if (result == 1 && packet == nullptr)
			continue;

		// If result is 0, the stream has reached eof, can just seek back to the beginning.
		if (result == 0)
		{
			result = nestegg_track_seek(state.context, state.trackIndex, 0);
			if (result == -1)
			{
				TS_PRINTF("  Track seek error.\n");
				errorText = "Track seek error.";
				processingResult = Error;
				break;
			}

			numFrames = 0;
			continue;
		}

		if (result < 0)
		{
			TS_PRINTF("  Packet read error.\n");
			errorText = "Stream packet read error.";
			processingResult = Error;
			break;
		}

		uint32 trackIndex = 0;
		result = nestegg_packet_track(packet, &trackIndex);
		TS_ASSERT(result == 0);

		if (trackIndex != state.trackIndex)
		{
// 			TS_PRINTF("Skipping packet: wrong track index (expected %u, got %u)\n", state.trackIndex, trackIndex);
			continue;
		}

		TS_ASSERT(nestegg_track_type(state.context, trackIndex) == NESTEGG_TRACK_VIDEO);

		if (nestegg_track_type(state.context, trackIndex) == NESTEGG_TRACK_VIDEO)
		{
// 			if (numFrames == 0)
// 			{
// 				uint64 scale = 0;
// 				result = nestegg_tstamp_scale(state.context, &scale);
// 				TS_ASSERT(result == 0);
// 				TS_PRINTF("Tstamp scale %llu\n", scale);
// 
// 				uint64 tstamp = 0;
// 				nestegg_packet_tstamp(packet, &tstamp);
// 				TS_ASSERT(result == 0);
// 
// 				TS_PRINTF("Packet timestamp %llu\n", tstamp / scale);
// 			}

			//TS_PRINTF("video frame: " << video_count << " ";
			uint32 packetCount = 0;
			result = nestegg_packet_count(packet, &packetCount);
			TS_ASSERT(result == 0);

			for (SizeType packetIndex = 0; packetIndex < packetCount; ++packetIndex)
			{
				uint8_t *data = nullptr;
				size_t length = 0;
				result = nestegg_packet_data(packet, packetIndex, &data, &length);
				TS_ASSERT(result == 0);
				if (result == -1)
				{
					errorText = "Stream packet read error.";
					processingResult = Error;
					break;
				}

				vpx_codec_stream_info_t streamInfo;
				memset(&streamInfo, 0, sizeof(streamInfo));
				streamInfo.sz = sizeof(streamInfo);

				vpx_codec_err_t error;
				error = vpx_codec_get_stream_info(state.codec, &streamInfo);
				if (error != VPX_CODEC_OK)
				{
					TS_PRINTF("vpx_codec_get_stream_info failed. error: %s (%d)\n", vpx_codec_err_to_string(error), error);
				}

				error = vpx_codec_peek_stream_info(state.interface, data, (uint32)length, &streamInfo);
				if (error != VPX_CODEC_OK)
				{
// 					TS_PRINTF("vpx_codec_peek_stream_info failed. error: %s (%d)\n", vpx_codec_err_to_string(error), error);
// 					processingResult = Error;
// 					break;
				}

				// Decode the frame
				error = vpx_codec_decode(state.codec, data, (uint32)length, nullptr, 0);
				if (error != VPX_CODEC_OK)
				{
					TS_PRINTF("vpx_codec_decode failed. error: %s (%d)\n", vpx_codec_err_to_string(error), error);
					errorText = "Video stream is corrupted.";
					processingResult = Error;
					break;
				}

				vpx_codec_iter_t iter = nullptr;
				while (vpx_image_t *image = vpx_codec_get_frame(state.codec, &iter))
				{
// 					TS_PRINTF("  numFrames %u / %u\n", ++numFrames, numTotalFrames);

					std::vector<Byte> framedata;
					framedata.resize(image->d_w * image->d_h * 4, 255);

					uint32 i = 0;
					for (uint32 y = 0; y < image->d_h; ++y)
					{
						for (uint32 x = 0; x < image->d_w; ++x)
						{
							framedata[i + 0] = image->planes[VPX_PLANE_Y][y       * image->stride[VPX_PLANE_Y] + x];

							if ((image->fmt & VPX_IMG_FMT_UV_FLIP) == 0)
							{
								framedata[i + 1] = image->planes[VPX_PLANE_U][(y / 2) * image->stride[VPX_PLANE_U] + (x / 2)];
								framedata[i + 2] = image->planes[VPX_PLANE_V][(y / 2) * image->stride[VPX_PLANE_V] + (x / 2)];
							}
							else
							{
								framedata[i + 1] = image->planes[VPX_PLANE_V][(y / 2) * image->stride[VPX_PLANE_V] + (x / 2)];
								framedata[i + 2] = image->planes[VPX_PLANE_U][(y / 2) * image->stride[VPX_PLANE_U] + (x / 2)];
							}

							if ((image->fmt & VPX_IMG_FMT_HAS_ALPHA) > 0)
								framedata[i + 3] = image->planes[VPX_PLANE_ALPHA][y * image->stride[VPX_PLANE_ALPHA] + x];
// 							else
// 								framedata[i + 3] = 255;

							i += 4;
						}
					}

					BufferedFrame frame;

					frame.texture = makeShared<sf::Texture>();
					if (frame.texture == nullptr)
					{
						processingResult = Error;
						errorText = "Failed to create texture.";
						break;
					}

					frame.texture->create(image->d_w, image->d_h);
					frame.texture->update(&framedata[0]);
					
					frame.texture->setRepeated(true);
					frame.texture->setSmooth(true);

					frame.frameTime = frameTime;

					bufferedFrames.push_back(std::move(frame));
				}

				if (processingResult != Undefined)
					break;
			}

			if (processingResult == Skipping)
				continue;

			if (processingResult == Error)
				break;
		}

		processingResult = Success;
		break;
	}

	TS_ASSERT(processingResult != Undefined && processingResult != Skipping);

	if (processingResult == Error)
	{
		TS_PRINTF("Processing result error!\n");
		return false;
	}

	if (!bufferedFrames.empty())
	{
		BufferedFrame &top = bufferedFrames.front();
		bufferStorage.texture = top.texture;
		bufferStorage.frameTime = top.frameTime;
		bufferedFrames.pop_front();
		return true;
	}

	TS_PRINTF("Error or out of frames!\n");

	return false;
}

bool ImageBackgroundLoaderWebm::isLoadingComplete() const
{
	return BaseClass::isLoadingComplete() && loaderIsComplete;
}

bool ImageBackgroundLoaderWebm::isValidWebmFile(const String &filepath)
{
	const BigSizeType nesteggSniffBytesAmount = 512;

	file::InputFile file(filepath, file::InputFileMode_ReadBinary);

	Byte buffer[nesteggSniffBytesAmount] = { 0 };
	file.read(&buffer[0], nesteggSniffBytesAmount);
	file.close();

	return nestegg_sniff(&buffer[0], nesteggSniffBytesAmount) == 1;
}

bool ImageBackgroundLoaderWebm::restartImpl(bool *shouldRestart)
{
	TS_ASSERT(shouldRestart != nullptr);

	if (nestegg_track_seek(state.context, state.trackIndex, 0) == -1)
	{
		errorText = "Track seek error.";
		return false;
	}

	*shouldRestart = true;

	numFrames = 0;
	return true;
}

bool ImageBackgroundLoaderWebm::loadNextFrame(FrameStorage &bufferStorage)
{
	TS_ASSERT(loaderIsComplete == false && "Is already complete.");
	if (loaderIsComplete || loaderState != Running)
		return false;

	if (!loaderIsPrepared)
	{
		bool success = prepareForLoading();
		if (!success)
		{
			cleanup();
			return false;
		}
	}

	bool success = processNextFrame(bufferStorage);
	if (!success)
	{
		cleanup();
		return false;
	}

	if (!ownerImage->active)
	{
		Thread::sleep(100_ms);
	}

	if (imageDataUpdated == false)
	{
		TS_ASSERT(imageData.size.x > 0 && imageData.size.y > 0 && "Image size is not set.");
		ownerImage->setImageData(imageData);
		imageDataUpdated = true;
	}

	return true;
}

bool ImageBackgroundLoaderWebm::wasLoadingCompleted() const
{
	return loaderIsComplete;
}

TS_END_PACKAGE2()


