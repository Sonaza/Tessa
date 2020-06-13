#include "Precompiled.h"
#include "SupportedFormats.h"

TS_PACKAGE2(app, viewer)

SupportedFormats SupportedFormats::instance;

const std::vector<String> &SupportedFormats::getSupportedFormatExtensions()
{
	return SupportedFormats::instance.supportedFormatExtensions;
}

bool SupportedFormats::hasAnimationSupport(const String &extension)
{
	FormatInfoList::const_iterator it = SupportedFormats::instance.supportedFormatInfo.find(extension);
	return it != SupportedFormats::instance.supportedFormatInfo.end() ? it->second.animated : false;
}

bool SupportedFormats::hasAlphaSupport(const String &extension)
{
	FormatInfoList::const_iterator it = SupportedFormats::instance.supportedFormatInfo.find(extension);
	return it != SupportedFormats::instance.supportedFormatInfo.end() ? it->second.supportsAlpha : false;
}

SupportedFormats::SupportedFormats()
{
	supportedFormatInfo = FormatInfoList
	{
		// BMP files
		{ "bmp",   { false, false } },
		// Dr. Halo CUT files (grayscale only)
		{ "cut",   { false, false } },
		// DDS files
		{ "dds",   { false, true } },
		// EXR files
		{ "exr",   { false, true } },
		// Raw Fax G3 files
		{ "g3",    { false, false } },
		// GIF files (animated)
		{ "gif",   { true, true } },
		// HDR files
		{ "hdr",   { false, true } },
		// ICO files (can support multiple layers)
		{ "ico",   { false, true } },
		// IFF files
		{ "iff",   { false, true } },
		// JNG files
		{ "jng",   { false, false } },
		// JPEG/JIF files
		{ "jpeg",  { false, false } },
		{ "jpg",   { false, false } },
		{ "jpe",   { false, false } },
		{ "jif",   { false, false } },
		{ "jfif",  { false, false } },
		{ "jfi",   { false, false } },
		// JPEG-2000 File Format
		{ "jpf",   { false, false } },
		{ "jpx",   { false, false } },
		{ "jp2",   { false, false } },
		{ "jpm",   { false, false } },
		{ "mj2",   { false, false } },
		// JPEG-2000 codestream
		{ "j2c",   { false, false } },
		{ "j2k",   { false, false } },
		{ "jpc",   { false, false } },
		// JPEG-XR files
		{ "jxr",   { false, false } },
		{ "hdp",   { false, false } },
		{ "wdp",   { false, false } },
		// KOALA files (http://fileformats.archiveteam.org/wiki/Atari_graphics_formats)
		{ "pic",   { false, false } },
		// Kodak PhotoCD files
		{ "pcd",   { false, false } },
		// MNG files (sort of an animated "PNG")
		{ "mng",   { true, true } },
		// PCX files
		{ "pcx",   { false, true } },
		{ "pcc",   { false, true } },
		// PBM/PGM/PPM files
		{ "pbm",   { false, true } },
		{ "pgm",   { false, true } },
		{ "ppm",   { false, true } },
		{ "pnm",   { false, true } },
		// PFM files
		{ "pfm",   { false, true } },
		// PNG files
		{ "png",   { false, true } },
		// Macintosh PICT files
		{ "pict",  { false, true } },
		{ "pct",   { false, true } },
		// Photoshop PSD files
		{ "psd",   { false, true } },
		// RAW camera files
		{ "raw",   { false, true } },
		// Sun RAS files
		{ "sun",   { false, true } },
		{ "ras",   { false, true } },
		{ "rast",  { false, true } },
		{ "rs",    { false, true } },
		{ "sr",    { false, true } },
		{ "scr",   { false, true } },
		{ "im1",   { false, true } },
		{ "im8",   { false, true } },
		{ "im24",  { false, true } },
		{ "im32",  { false, true } },
		// SGI files
		{ "sgi",   { false, true } },
		// TARGA files
		{ "tga",   { false, true } },
		{ "icb",   { false, true } },
		{ "vda",   { false, true } },
		{ "vst",   { false, true } },
		// TIFF files
		{ "tif",   { false, true } },
		{ "tiff",  { false, true } },
		// WBMP files
		{ "wbmp",  { false, true } },
		// WebP files
		{ "webp",  { false, true } },
		// XBM files
		{ "xbm",   { false, true } },
		{ "bm",    { false, true } },
		// XPM files
		{ "xpm",   { false, true } },
		{ "pm",    { false, true } },

		// WEBM video files
		{ "webm",  { true, false } }
	};

	supportedFormatExtensions.reserve(supportedFormatInfo.size());
	for (auto &it : supportedFormatInfo)
	{
		supportedFormatExtensions.push_back(it.first);
	}
}

TS_END_PACKAGE2()


