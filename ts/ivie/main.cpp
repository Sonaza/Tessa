#include "Precompiled.h"
#include "Application.h"

#include <locale>
#include <codecvt>
#include <fstream>

#include "ts/tessa/thread/Thread.h"
#include "ts/tessa/common/Log.h"
#include "ts/tessa/file/FileUtils.h"

#include "ts/tessa/lang/Signal.h"

using namespace ts;

int asdfasd()
{
// 	lang::Signal<> mysignal;
// 	lang::SignalBind bind2;
// 	bind2.connect(mysignal, lang::SignalPriority_Normal, []()
// 	{
// 		TS_PRINTF("This is Bind 2 callback!\n");
// 	});

// 	lang::SignalBind bind1;
// 	bind1.connect(mysignal, lang::SignalPriority_Normal, &bind1function);
	

// 	mysignal();

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