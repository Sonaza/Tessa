#include "Precompiled.h"

#include "ts/tessa/file/ArchivistWriter.h"

using namespace ts;

void makepack()
{
	file::ArchivistWriter aw;

	std::vector<String> files = {
		"shader/area_clip.frag",
		"shader/background_gradient.frag",
		"shader/convert_freeimage.frag",
		"shader/convert_webm.frag",
		"SourceHanSans-Medium.ttc",
		"selawk.ttf",
		"ivie_logo_32.png",
	};

	for (const String &f : files)
	{
		aw.stageFile(f, f, file::CompressionType_LZ4FullBlock);
	}

	if (!aw.saveToFile("resources.tsp"))
	{
		TS_LOG_ERROR("Failed to write resource pack.");
	}
}
