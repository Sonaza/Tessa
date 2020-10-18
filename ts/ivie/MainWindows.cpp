#include "Precompiled.h"

#if TS_PLATFORM == TS_WINDOWS

#include "Application.h"

#include "ts/lang/common/IncludeWindows.h"
#include <shellapi.h>

#include "ts/engine/system/Commando.h"

#include "zip.h"
#include "ts/file/OutputFile.h"

using namespace ts;

extern void makepack();

void mathtest()
{
	math::Mat4 mt;
	math::Quat q = math::Quat::makeFromRotation(math::VC3::up, math::VC3::forward);
	q.normalize().getMatrix(mt);

	math::VC3 euler = q.getEulerAngles();
}

void zippy()
{
	int error = 0;
	zip_t *archive = zip_open("nana.cbz", 0, &error);
	if (!archive)
	{
		char buf[100];
		zip_error_to_str(buf, sizeof(buf), error, errno);
		TS_PRINTF("can't open zip archive: %s", buf);
		return;
	}

	zip_stat_t sb;
	for (int i = 0; i < zip_get_num_entries(archive, 0); i++)
	{
		if (zip_stat_index(archive, i, 0, &sb) == 0)
		{
			TS_PRINTF("Name: [%s], ", sb.name);
			TS_PRINTF("Size: [%llu], ", sb.size);
			TS_PRINTF("mtime: [%u]\n\n", (unsigned int)sb.mtime);

			int len = (int)strlen(sb.name);
			if (sb.name[len - 1] == '/')
			{
				continue;
			}
			else
			{
				zip_file_t *zf = zip_fopen_index(archive, i, 0);
				if (!zf)
				{
					TS_PRINTF("Failed to open file from archive.\n");
					return;
				}

				file::OutputFile out;
				if (!out.open(TS_FMT("nanazip/%s", sb.name), file::OutputFileMode_WriteBinary))
				{
					TS_PRINTF("Failed to open file for writing.\n");
					return;
				}

				char buf[1024];
				zip_uint64_t bytesReadTotal = 0;
				while (bytesReadTotal != sb.size)
				{
					zip_int64_t bytesRead = zip_fread(zf, buf, 1024);
					if (bytesRead < 0)
					{
						TS_PRINTF("Failed zip read.\n");
						return;
					}

					out.write(buf, (uint32)bytesRead);
					bytesReadTotal += bytesRead;
				}
				out.close();
				zip_fclose(zf);
			}
		}
	}

	zip_close(archive);
}

// int wmain(int argc, const wchar_t **argv)
int __stdcall wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR cmdArgs, int windowShowCmd)
{
// 	makepack();
// 	mathtest();
// 	zippy();
	
	engine::system::Commando commando;
	
	int32 numArgs = 0;
	LPWSTR *argList = CommandLineToArgvW(cmdArgs, &numArgs);
	if (argList != nullptr && numArgs > 0)
	{
		// Gotta const cast here because parser expects const parameter.
		const wchar_t **argv = const_cast<const wchar_t **>(argList);

		if (!commando.parse(numArgs, argv))
			TS_LOG_WARNING("Failed to parse command line arguments.");

		LocalFree(argList);
	}

	app::Application app(commando);
	int returnCode = app.launch();
	return returnCode;
}

#endif
