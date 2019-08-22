#pragma once

#include <ivie/ivie/image/BaseImageLoader.hpp>
#include <string>
#include <memory>

TS_PACKAGE2(app, viewer)

class ImageLoaderWebM : public BaseImageLoader
{
public:
	ImageLoaderWebM();
	virtual ~ImageLoaderWebM();
	
	virtual bool beginLoadFromFile(const std::wstring& path, Image *outImage);

private:
	class Impl;
	std::unique_ptr<Impl> impl;
};

TS_END_PACKAGE2()
