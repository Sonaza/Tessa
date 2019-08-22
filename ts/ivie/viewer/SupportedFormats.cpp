#include "Precompiled.h"
#include "SupportedFormats.h"

TS_PACKAGE2(app, viewer)

SupportedFormats SupportedFormats::instance;

const std::vector<std::wstring> &SupportedFormats::getSupportedFormatExtensions()
{
	return SupportedFormats::instance.supportedFormatExtensions;
}

const bool SupportedFormats::isFormatAnimated(const std::wstring &extension)
{
	FormatInfoList::const_iterator it = SupportedFormats::instance.supportedFormatInfo.find(extension);
	return it != SupportedFormats::instance.supportedFormatInfo.end() ? it->second.animated : false;
}

SupportedFormats::SupportedFormats()
{
	supportedFormatInfo = FormatInfoList
	{
		// BMP files
		{ L"bmp",   { false } },
		// Dr. Halo CUT files (grayscale only)
		{ L"cut",   { false } },
		// DDS files
		{ L"dds",   { false } },
		// EXR files
		{ L"exr",   { false } },
		// Raw Fax G3 files
		{ L"g3",    { false } },
		// GIF files (animated)
		{ L"gif",   { true } },
		// HDR files
		{ L"hdr",   { false } },
		// ICO files (can support multiple layers)
		{ L"ico",   { false } },
		// IFF files
		{ L"iff",   { false } },
		// JNG files
		{ L"jng",   { false } },
		// JPEG/JIF files
		{ L"jpeg",  { false } },
		{ L"jpg",   { false } },
		{ L"jpe",   { false } },
		{ L"jif",   { false } },
		{ L"jfif",  { false } },
		{ L"jfi",   { false } },
		// JPEG-2000 File Format
		{ L"jpf",   { false } },
		{ L"jpx",   { false } },
		{ L"jp2",   { false } },
		{ L"jpm",   { false } },
		{ L"mj2",   { false } },
		// JPEG-2000 codestream
		{ L"j2c",   { false } },
		{ L"j2k",   { false } },
		{ L"jpc",   { false } },
		// JPEG-XR files
		{ L"jxr",   { false } },
		{ L"hdp",   { false } },
		{ L"wdp",   { false } },
		// KOALA files (http://fileformats.archiveteam.org/wiki/Atari_graphics_formats)
		{ L"pic",   { false } },
		// Kodak PhotoCD files
		{ L"pcd",   { false } },
		// MNG files (sort of an animated "PNG")
		{ L"mng",   { true } },
		// PCX files
		{ L"pcx",   { false } },
		{ L"pcc",   { false } },
		// PBM/PGM/PPM files
		{ L"pbm",   { false } },
		{ L"pgm",   { false } },
		{ L"ppm",   { false } },
		{ L"pnm",   { false } },
		// PFM files
		{ L"pfm",   { false } },
		// PNG files
		{ L"png",   { false } },
		// Macintosh PICT files
		{ L"pict",  { false } },
		{ L"pct",   { false } },
		// Photoshop PSD files
		{ L"psd",   { false } },
		// RAW camera files
		{ L"raw",   { false } },
		// Sun RAS files
		{ L"sun",   { false } },
		{ L"ras",   { false } },
		{ L"rast",  { false } },
		{ L"rs",    { false } },
		{ L"sr",    { false } },
		{ L"scr",   { false } },
		{ L"im1",   { false } },
		{ L"im8",   { false } },
		{ L"im24",  { false } },
		{ L"im32",  { false } },
		// SGI files
		{ L"sgi",   { false } },
		// TARGA files
		{ L"tga",   { false } },
		{ L"icb",   { false } },
		{ L"vda",   { false } },
		{ L"vst",   { false } },
		// TIFF files
		{ L"tif",   { false } },
		{ L"tiff",  { false } },
		// WBMP files
		{ L"wbmp",  { false } },
		// WebP files
		{ L"webp",  { false } },
		// XBM files
		{ L"xbm",   { false } },
		{ L"bm",    { false } },
		// XPM files
		{ L"xpm",   { false } },
		{ L"pm",    { false } },

		// WEBM video files
// 		{ L"webm",  { true } }
	};

	supportedFormatExtensions.reserve(supportedFormatInfo.size());
	for (auto &it : supportedFormatInfo)
	{
		supportedFormatExtensions.push_back(it.first);
	}
}

TS_END_PACKAGE2()


