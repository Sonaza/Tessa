#include "Precompiled.h"
#include "Application.h"

#include <locale>
#include <codecvt>
#include <fstream>

#include "ts/tessa/thread/Thread.h"
#include "ts/tessa/common/Log.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/ivie/util/RingBuffer.h"

#include <functional>
#include <future>

using namespace ts;

int asdfasd()
{
	

	return 1;
}

int wmain(int argc, const wchar_t **argv)
{
// 	if (asdfasd())
// 		return 0;

	thread::Thread::setMainThread(thread::Thread::getCurrentThread());
	
	app::Application app(argc, argv);
	int returnCode = app.launch();
	return returnCode;
}