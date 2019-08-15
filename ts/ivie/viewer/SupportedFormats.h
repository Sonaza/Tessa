#pragma once

#include <vector>
#include <string>

TS_PACKAGE2(app, viewer)

// struct FormatInfo
// {
// 	std::string extension;
// };

class SupportedFormats
{
public:
	static const std::vector<std::wstring> &getFormats();

private:
	static SupportedFormats instance;
	SupportedFormats();

	std::vector<std::wstring> supportedFormats;
};

TS_END_PACKAGE2()
