#include "Precompiled.h"
#include "SupportedFormats.h"

TS_PACKAGE2(app, viewer)

SupportedFormats SupportedFormats::instance;

const std::vector<String> &SupportedFormats::getSupportedFormatExtensions()
{
	return SupportedFormats::instance.supportedFormatExtensions;
}

bool SupportedFormats::isFormatAnimated(const String &extension)
{
	FormatInfoList::const_iterator it = SupportedFormats::instance.supportedFormatInfo.find(extension);
	return it != SupportedFormats::instance.supportedFormatInfo.end() ? it->second.animated : false;
}

SupportedFormats::SupportedFormats()
{
	supportedFormatInfo = FormatInfoList
	{
		// BMP files
		{ "bmp",   { false } },
		// Dr. Halo CUT files (grayscale only)
		{ "cut",   { false } },
		// DDS files
		{ "dds",   { false } },
		// EXR files
		{ "exr",   { false } },
		// Raw Fax G3 files
		{ "g3",    { false } },
		// GIF files (animated)
		{ "gif",   { true } },
		// HDR files
		{ "hdr",   { false } },
		// ICO files (can support multiple layers)
		{ "ico",   { false } },
		// IFF files
		{ "iff",   { false } },
		// JNG files
		{ "jng",   { false } },
		// JPEG/JIF files
		{ "jpeg",  { false } },
		{ "jpg",   { false } },
		{ "jpe",   { false } },
		{ "jif",   { false } },
		{ "jfif",  { false } },
		{ "jfi",   { false } },
		// JPEG-2000 File Format
		{ "jpf",   { false } },
		{ "jpx",   { false } },
		{ "jp2",   { false } },
		{ "jpm",   { false } },
		{ "mj2",   { false } },
		// JPEG-2000 codestream
		{ "j2c",   { false } },
		{ "j2k",   { false } },
		{ "jpc",   { false } },
		// JPEG-XR files
		{ "jxr",   { false } },
		{ "hdp",   { false } },
		{ "wdp",   { false } },
		// KOALA files (http://fileformats.archiveteam.org/wiki/Atari_graphics_formats)
		{ "pic",   { false } },
		// Kodak PhotoCD files
		{ "pcd",   { false } },
		// MNG files (sort of an animated "PNG")
		{ "mng",   { true } },
		// PCX files
		{ "pcx",   { false } },
		{ "pcc",   { false } },
		// PBM/PGM/PPM files
		{ "pbm",   { false } },
		{ "pgm",   { false } },
		{ "ppm",   { false } },
		{ "pnm",   { false } },
		// PFM files
		{ "pfm",   { false } },
		// PNG files
		{ "png",   { false } },
		// Macintosh PICT files
		{ "pict",  { false } },
		{ "pct",   { false } },
		// Photoshop PSD files
		{ "psd",   { false } },
		// RAW camera files
		{ "raw",   { false } },
		// Sun RAS files
		{ "sun",   { false } },
		{ "ras",   { false } },
		{ "rast",  { false } },
		{ "rs",    { false } },
		{ "sr",    { false } },
		{ "scr",   { false } },
		{ "im1",   { false } },
		{ "im8",   { false } },
		{ "im24",  { false } },
		{ "im32",  { false } },
		// SGI files
		{ "sgi",   { false } },
		// TARGA files
		{ "tga",   { false } },
		{ "icb",   { false } },
		{ "vda",   { false } },
		{ "vst",   { false } },
		// TIFF files
		{ "tif",   { false } },
		{ "tiff",  { false } },
		// WBMP files
		{ "wbmp",  { false } },
		// WebP files
		{ "webp",  { false } },
		// XBM files
		{ "xbm",   { false } },
		{ "bm",    { false } },
		// XPM files
		{ "xpm",   { false } },
		{ "pm",    { false } },

		// WEBM video files
		{ "webm",  { true } }
	};

	supportedFormatExtensions.reserve(supportedFormatInfo.size());
	for (auto &it : supportedFormatInfo)
	{
		supportedFormatExtensions.push_back(it.first);
	}
}

TS_END_PACKAGE2()


