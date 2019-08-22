#include <ivie/ivie/Precompiled.h>

#include <ivie/ivie/image/ImageLoaderWebM.hpp>
#include <ivie/ivie/image/Image.hpp>
#include <ivie/sys/file/InputFile.hpp>

#include <SFML/Graphics.hpp>
#include <ivie/ivie/image/webm_include.hpp>

#include <fstream>

TS_PACKAGE2(app, viewer)

namespace
{

sf::VertexArray makeQuadVertexArray(uint32_t width, uint32_t height)
{
	sf::VertexArray va(sf::Quads, 4);
	va[0] = sf::Vertex(
		sf::Vector2f(0.f, 0.f),
		sf::Vector2f(0.f, 0.f)
		);
	va[1] = sf::Vertex(
		sf::Vector2f(static_cast<float>(width), 0.f),
		sf::Vector2f(static_cast<float>(width), 0.f)
		);
	va[2] = sf::Vertex(
		sf::Vector2f(static_cast<float>(width), static_cast<float>(height)),
		sf::Vector2f(static_cast<float>(width), static_cast<float>(height))
		);
	va[3] = sf::Vertex(
		sf::Vector2f(0.f, static_cast<float>(height)),
		sf::Vector2f(0.f, static_cast<float>(height))
		);
	return va;
}

int ifstream_read(void *buffer, size_t size, void *context)
{
	std::ifstream* f = (std::ifstream*)context;
	f->read((char*)buffer, size);
	// success = 1
	// eof = 0
	// error = -1
	return f->gcount() == size ? 1 : f->eof() ? 0 : -1;
}

int ifstream_seek(int64_t n, int whence, void *context)
{
	std::ifstream* f = (std::ifstream*)context;
	f->clear();
	std::ios_base::seekdir dir;
	switch(whence)
	{
		case NESTEGG_SEEK_SET:
			dir = std::ifstream::beg;
			break;
		case NESTEGG_SEEK_CUR:
			dir = std::ifstream::cur;
			break;
		case NESTEGG_SEEK_END:
			dir = std::ifstream::end;
			break;
	}
	f->seekg(n, dir);
	if(!f->good())
		return -1;
	return 0;
}

int64_t ifstream_tell(void* context)
{
	std::ifstream* f = (std::ifstream*)context;
	return f->tellg();
}

void log_nestegg(nestegg * context, unsigned int severity, char const* format, ...)
{
	va_list args;
	va_start(args, format);
	log::printf(format, args);
	va_end(args);
	IV_PRINTF("\n");
}

}

struct ProcessedImage
{
	std::vector<Frame> frames;
	uint32_t width;
	uint32_t height;
};

class ImageLoaderWebM::Impl
{
public:
	Impl()
	{

	}

	~Impl()
	{

	}

	int32_t clamp8(int32_t value)
	{
		return value >= 0 ? (value <= 255 ? value : 255) : 0;
	}

	void convertYV12toRGBA(const vpx_image_t* img, std::vector<uint8_t>& data)
	{
		IV_ASSERT(img != NULL);

		data.resize(img->d_w * img->d_h * 4, 255);

		uint8_t *yPlane = img->planes[VPX_PLANE_Y];
		uint8_t *uPlane = img->planes[VPX_PLANE_U];
		uint8_t *vPlane = img->planes[VPX_PLANE_V];

		int i = 0;
		for(unsigned int imgY = 0; imgY < img->d_h; imgY++)
		{
			for(unsigned int imgX = 0; imgX < img->d_w; imgX++)
			{
				int y = yPlane[ imgY      * img->stride[VPX_PLANE_Y] +  imgX];
				int u = uPlane[(imgY / 2) * img->stride[VPX_PLANE_U] + (imgX / 2)];
				int v = vPlane[(imgY / 2) * img->stride[VPX_PLANE_V] + (imgX / 2)];

				int c = y - 16;
				int d = u - 128;
				int e = v - 128;

				int r = clamp8((298 * c + 409 * e + 128) >> 8);
				int g = clamp8((298 * c - 100 * d - 208 * e + 128) >> 8);
				int b = clamp8((298 * c + 516 * d + 128) >> 8);

				data[i + 0] = static_cast<uint8_t>(r);
				data[i + 1] = static_cast<uint8_t>(g);
				data[i + 2] = static_cast<uint8_t>(b);
				data[i + 3] = 255;

				i += 4;
			}
		}
	}

	bool loadFromFile(const std::wstring& path, Image *outImage)
	{
		IV_ASSERT(!path.empty());

		int result;

		std::ifstream file(path, std::ifstream::binary);
		IV_ASSERT(file.is_open());

		nestegg_io ne_io;
		ne_io.read = ifstream_read;
		ne_io.seek = ifstream_seek;
		ne_io.tell = ifstream_tell;
		ne_io.userdata = (void*)&file;

		nestegg* context = NULL;
		result = nestegg_init(&context, ne_io, NULL, -1);
		IV_PRINTF("nestegg_init = %d\n", result);
		IV_ASSERT(result == 0);
		IV_ASSERT(context);

		uint64_t total_duration = 0;
		result = nestegg_duration(context, &total_duration);
		IV_ASSERT(result == 0);
		IV_PRINTF("Duration %u\n", total_duration);

		unsigned int num_tracks = 0;
		result = nestegg_track_count(context, &num_tracks);
		IV_ASSERT(result == 0);
		IV_PRINTF("Tracks: %u\n", num_tracks);

		nestegg_video_params vparams;
		vparams.width = 0;
		vparams.height = 0;

		bool has_video_track = false;

		ProcessedImage processedImage;
		uint32_t frametime = 0;

		vpx_codec_iface_t* interface = NULL;
		for(unsigned int track = 0; track < num_tracks; ++track)
		{
			int id = nestegg_track_codec_id(context, track);
			IV_ASSERT(id >= 0);

			int type = nestegg_track_type(context, track);
			IV_PRINTF("Track %u codec id: %d type %d\n", track, id, type);

			interface = id == NESTEGG_CODEC_VP9 ? &vpx_codec_vp9_dx_algo : &vpx_codec_vp8_dx_algo;
			if(type == NESTEGG_TRACK_VIDEO)
			{
				result = nestegg_track_video_params(context, track, &vparams);
				IV_ASSERT(result == 0);

				uint64_t duration = 0;
				result = nestegg_track_default_duration(context, track, &duration);
				IV_ASSERT(result == 0);

				// TODO: Fix! It's not actually accurate
				uint32_t numFrames = static_cast<uint32_t>(total_duration / duration);
				IV_PRINTF("Video has about %u frames\n", numFrames);

				// Convert nanoseconds to milliseconds
				frametime = static_cast<uint32_t>(duration / 1000000);

				IV_PRINTF("Frame time: %u\nFPS: %.2f\n", frametime, (1000.f / (float)frametime));
				IV_PRINTF("Size: %ux%u\nDisplay size: %ux%u\n", vparams.width, vparams.height, vparams.display_width, vparams.display_height);

				has_video_track = true;
				break;
			}
		}

		if(!has_video_track)
		{
			IV_PRINTF("File does not have video track\n");
			return false;
		}

		processedImage.width = vparams.display_width;
		processedImage.height = vparams.display_height;

		vpx_codec_ctx_t  codec;
		int              flags = 0;

		IV_PRINTF("Using %s\n", vpx_codec_iface_name(interface));;

		/* Initialize codec */
		if(vpx_codec_dec_init(&codec, interface, NULL, flags))
		{
			IV_PRINTF("Failed to initialize decoder\n");
			return false;
		}

		nestegg_packet* packet = 0;

		while(1)
		{
			result = nestegg_read_packet(context, &packet);
			if(result == 1 && packet == 0) continue;
			if(result <= 0) break;

			uint32_t track = 0;
			result = nestegg_packet_track(packet, &track);
			IV_ASSERT(result == 0);

			// TODO: workaround bug
			if(nestegg_track_type(context, track) == NESTEGG_TRACK_VIDEO)
			{
				//IV_PRINTF("video frame: " << video_count << " ";
				uint32_t packet_count = 0;
				result = nestegg_packet_count(packet, &packet_count);
				IV_ASSERT(result == 0);

				//IV_PRINTF("Count: " << count << " ";
				size_t num_frames = 0;
				for(size_t packet_index = 0; packet_index < packet_count; ++packet_index)
				{
					uint8_t* data;
					size_t length;
					result = nestegg_packet_data(packet, static_cast<uint32_t>(packet_index), &data, &length);
					IV_ASSERT(result == 0);

					vpx_codec_stream_info_t streamInfo;
					memset(&streamInfo, 0, sizeof(streamInfo));
					streamInfo.sz = sizeof(streamInfo);
					vpx_codec_peek_stream_info(interface, data, (uint32_t)length, &streamInfo);

					// Decode the frame
					vpx_codec_err_t codecError = vpx_codec_decode(&codec, data, (uint32_t)length, NULL, 0);
					if(codecError != VPX_CODEC_OK)
					{
						IV_PRINTF("Failed to decode frame. error: %d\n", codecError);
						return false;
					}

					vpx_codec_iter_t  iter = NULL;
					while(vpx_image_t* img = vpx_codec_get_frame(&codec, &iter))
					{
// 						IV_PRINTF("Format: ", img->fmt);

						std::vector<uint8_t> data;
						data.resize(img->d_w * img->d_h * 4);

						uint32_t i = 0;
						for(uint32_t y = 0; y < img->d_h; ++y)
						{
							for(uint32_t x = 0; x < img->d_w; ++x)
							{
								data[i + 0] = img->planes[VPX_PLANE_Y][y       * img->stride[VPX_PLANE_Y] + x];
								data[i + 1] = img->planes[VPX_PLANE_U][(y / 2) * img->stride[VPX_PLANE_U] + (x / 2)];
								data[i + 2] = img->planes[VPX_PLANE_V][(y / 2) * img->stride[VPX_PLANE_V] + (x / 2)];
								data[i + 3] = 255;
								i += 4;
							}
						}

// 						convertYV12toRGBA(img, data);

						sf::Image image;
						image.create(img->d_w, img->d_h, &data[0]);

						processedImage.width = img->d_w;
						processedImage.height = img->d_h;

						sf::Texture* texture = new sf::Texture;
						texture->loadFromImage(image);

						texture->setRepeated(true);
						texture->setSmooth(true);

						processedImage.frames.push_back(Frame{ texture, frametime });
					}
				}
			}
		}

		IV_PRINTF("Processing finished\n");
		IV_PRINTF("  Total number of frames: %u\n", processedImage.frames.size());

		if(vpx_codec_destroy(&codec))
		{
			IV_PRINTF("Failed to destroy codec\n");
		}

		nestegg_destroy(context);
		file.close();

		if(outImage)
		{
			outImage->setRenderShaderType(Image::YUVtoRGB);
			outImage->setSize(processedImage.width, processedImage.height);
			outImage->setFrames(std::move(processedImage.frames));
			return true;
		}

		return false;
	}
};

ImageLoaderWebM::ImageLoaderWebM()
{
	impl = std::make_unique<Impl>();
	IV_ASSERT(impl);
}

ImageLoaderWebM::~ImageLoaderWebM()
{

}

bool ImageLoaderWebM::beginLoadFromFile(const std::wstring& path, Image *outImage)
{
	return impl->loadFromFile(path, outImage);
}

TS_END_PACKAGE2()
