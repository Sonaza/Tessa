#include "Precompiled.h"
#include "AppConfig.h"

TS_PACKAGE0()

String getApplicationName()
{
	return APP_NAME;
}

String getApplicationLogFile()
{
	return APP_LOG_FILE;
}

String getApplicationConfigFile()
{
	return APP_CONFIG_FILE;
}

extern String getApplicationDefaultWindowTitle()
{
	return APP_WINDOW_TITLE;
}

TS_END_PACKAGE0()
