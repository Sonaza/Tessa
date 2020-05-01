#include "Precompiled.h"

#if 0

#include "DisplayImage.h"

#include "ts/ivie/image/Image.h"

TS_PACKAGE2(app, image)

DisplayImage::DisplayImage()
{

}

DisplayImage::~DisplayImage()
{

}

void DisplayImage::update(TimeSpan deltatime)
{
	if (currentImage->getIsAnimated())
	{
		elapsedFrameTime += deltatime;
		if (elapsedFrameTime >= currentFrame->frameTime && currentImage->advanceToNextFrame())
		{
			elapsedFrameTime -= currentFrame->frameTime;
			currentFrame = currentImage->getCurrentFrameStorage();
		}
	}
}

void DisplayImage::setImage(SharedPointer<Image> image)
{
	currentImage = image;
	currentFrame = currentImage->getCurrentFrameStorage();
}

void DisplayImage::draw(sf::RenderTarget &renderTarget, const math::VC2 &position, float scale)
{

}

TS_END_PACKAGE2()

#endif
