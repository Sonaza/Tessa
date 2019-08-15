#include "Precompiled.h"
#include "SupportedFormats.h"

TS_PACKAGE2(app, viewer)

SupportedFormats SupportedFormats::instance;

const std::vector<std::wstring> &SupportedFormats::getFormats()
{
	return SupportedFormats::instance.supportedFormats;
}

SupportedFormats::SupportedFormats()
{
	supportedFormats = std::vector<std::wstring>
	{
		// BMP files
		L"bmp",
		// Dr. Halo CUT files (grayscale only)
		L"cut",
		// DDS files
		L"dds",
		// EXR files
		L"exr",
		// Raw Fax G3 files
		L"g3",
		// GIF files (animated)
		L"gif",
		// HDR files
		L"hdr",
		// ICO files (can support multiple layers)
		L"ico",
		// IFF files
		L"iff",
		// JNG files
		L"jng",
		// JPEG/JIF files
		L"jpeg",
		L"jpg",
		L"jpe",
		L"jif",
		L"jfif",
		L"jfi",
		// JPEG-2000 File Format
		L"jpf",
		L"jpx",
		L"jp2",
		L"jpm",
		L"mj2",
		// JPEG-2000 codestream
		L"j2c",
		L"j2k",
		L"jpc",
		// JPEG-XR files
		L"jxr",
		L"hdp",
		L"wdp",
		// KOALA files (http://fileformats.archiveteam.org/wiki/Atari_graphics_formats)
		L"pic",
		// Kodak PhotoCD files
		L"pcd",
		// MNG files (sort of an animated "PNG")
		L"mng",
		// PCX files
		L"pcx",
		L"pcc",
		// PBM/PGM/PPM files
		L"pbm",
		L"pgm",
		L"ppm",
		L"pnm",
		// PFM files
		L"pfm",
		// PNG files
		L"png",
		// Macintosh PICT files
		L"pict",
		L"pct",
		// Photoshop PSD files
		L"psd",
		// RAW camera files
		L"raw",
		// Sun RAS files
		L"sun",
		L"ras",
		L"rast",
		L"rs",
		L"sr",
		L"scr",
		L"im1",
		L"im8",
		L"im24",
		L"im32",
		// SGI files
		L"sgi",
		// TARGA files
		L"tga",
		L"icb",
		L"vda",
		L"vst",
		// TIFF files
		L"tif",
		L"tiff",
		// WBMP files
		L"wbmp",
		// WebP files
		L"webp",
		// XBM files
		L"xbm",
		L"bm",
		// XPM files
		L"xpm",
		L"pm",

		// WEBM video files
		// L"webm"
	};
}

TS_END_PACKAGE2()


