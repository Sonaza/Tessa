#pragma once

#include <vector>
#include <string>

TS_PACKAGE2(app, viewer)

struct FormatInfo
{
	bool animated;
};

class SupportedFormats
{
public:
	static const std::vector<String> &getSupportedFormatExtensions();

	static const bool isFormatAnimated(const String &extension);

private:
	static SupportedFormats instance;
	SupportedFormats();

	typedef std::map<String, FormatInfo> FormatInfoList;
	FormatInfoList supportedFormatInfo;

	std::vector<String> supportedFormatExtensions;
};

TS_END_PACKAGE2()
