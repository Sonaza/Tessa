#include "Precompiled.h"
#include "Application.h"

#include <locale>
#include <codecvt>
#include <fstream>

#include "ts/tessa/threading/Thread.h"
#include "ts/tessa/common/Log.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/ivie/util/RingBuffer.h"

#include <functional>
#include <future>

using namespace ts;

int asd()
{
	
	return 0;
}

int wmain(int argc, const wchar_t **argv)
{
// 	if (asd())
// 		return 0;

	threading::Thread::setMainThread(threading::Thread::getCurrentThread());
	
	app::Application app(argc, argv);
	int returnCode = app.launch();
	return returnCode;
}