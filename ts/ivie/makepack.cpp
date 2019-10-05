#include "Precompiled.h"

#include "ts/resource/archivist/ArchivistWriter.h"

using namespace ts;

void makepack()
{
	resource::archivist::ArchivistWriter aw;

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
		aw.stageFile(f, f, resource::archivist::CompressionType_LZ4FullBlock);
	}

	if (!aw.saveToFile("resources.tsp"))
	{
		TS_LOG_ERROR("Failed to write resource pack.");
	}
}
