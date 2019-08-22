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
	static const std::vector<std::wstring> &getSupportedFormatExtensions();

	static const bool isFormatAnimated(const std::wstring &extension);

private:
	static SupportedFormats instance;
	SupportedFormats();

	typedef std::map<std::wstring, FormatInfo> FormatInfoList;
	FormatInfoList supportedFormatInfo;

	std::vector<std::wstring> supportedFormatExtensions;
};

TS_END_PACKAGE2()
