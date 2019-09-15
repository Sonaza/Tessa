#pragma once

TS_DECLARE2(app, image, Image);

TS_PACKAGE2(app, image)

class DisplayImage
{
	typedef DisplayImage ThisClass;

public:
	DisplayImage();
	~DisplayImage();

	void update(TimeSpan deltatime);

	void setImage(SharedPointer<Image> image);

	void draw(sf::RenderTarget &renderTarget, const math::VC2 &position, float scale);

private:
	SharedPointer<Image> currentImage;
	TimeSpan elapsedFrameTime;

	const FrameStorage *currentFrame = nullptr;
// 	SharedPointer<Image> previousImage;
};

TS_END_PACKAGE2()
