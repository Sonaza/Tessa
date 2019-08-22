#include "Precompiled.h"
#include "ts/ivie/scenes/ImageViewerScene.h"

#include "ts/tessa/file/ArchivistFilesystem.h"
#include "ts/tessa/file/InputFile.h"
#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/file/FileListW.h"

#include "FreeImage.h"

#include "ts/ivie/util/NaturalSort.h"
#include "ts/ivie/viewer/BackgroundFileScanner.h"
#include "ts/ivie/viewer/ViewerStateManager.h"

#include "ts/ivie/viewer/image/ImageManager.h"
#include "ts/ivie/viewer/image/Image.h"

#include "ts/ivie/util/RenderUtil.h"

TS_PACKAGE2(app, scenes)

ImageViewerScene::ImageViewerScene(system::BaseApplication *application)
	: AbstractSceneBase(application)
{
}

ImageViewerScene::~ImageViewerScene()
{
}

bool ImageViewerScene::start()
{
	viewerStateManager = &getGigaton<viewer::ViewerStateManager>();

	imageChangedBind.connect(viewerStateManager->currentImageChangedSignal, &ThisClass::imageChanged, this);

	return true;
}

void ImageViewerScene::stop()
{
}

void ImageViewerScene::loadResources(resource::ResourceManager &rm)
{
	font = rm.loadResource<resource::FontResource>("viewer_font", "SourceHanSans-Medium.ttc", true);

	shader.loadFromFile("convert.frag", sf::Shader::Fragment);
}

bool ImageViewerScene::handleEvent(const sf::Event event)
{
	switch (event.type)
	{
		case sf::Event::KeyPressed:
		{
			if (event.key.code == sf::Keyboard::Left)
			{
				viewerStateManager->previousImage();
				return true;
			}
			if (event.key.code == sf::Keyboard::Right)
			{
				viewerStateManager->nextImage();
				return true;
			}
		}
		break;

		case sf::Event::MouseButtonPressed:
		{
			if (event.mouseButton.button == sf::Mouse::XButton1)
			{
				viewerStateManager->previousImage();
				return true;
			}
			if (event.mouseButton.button == sf::Mouse::XButton2)
			{
				viewerStateManager->nextImage();
				return true;
			}
		}
		break;
	}

	return false;
}

void ImageViewerScene::update(const TimeSpan deltaTime)
{
// 	system::WindowManager &wm = application->getManager<system::WindowManager>();
// 	math::VC2U windowSize = wm.getSize();
// 	float offset = sf::Mouse::getPosition(wm.getRenderWindow()).x / (float)windowSize.x;
// 
// 	float scale = (900.f + offset * 600.f) / (float)texSize.y;
// 	sprite.setScale(scale, scale);
}

void ImageViewerScene::imageChanged(SizeType statusText)
{
	frameTimer.restart();
}

void ImageViewerScene::render(sf::RenderWindow &renderWindow)
{
	viewer::ImageManager &imageManager = getGigaton<viewer::ImageManager>();
	
	SizeType frameIndex = 0;
	SizeType frameIndexMax = 1;
	SizeType framesBuffered = 0;

	viewer::Image *currentImage = imageManager.getCurrentImage();
	if (currentImage != nullptr && currentImage->isDisplayable())
	{
		frameIndex = currentImage->getCurrentFrameIndex();
		frameIndexMax = currentImage->getNumFramesTotal();
		framesBuffered = currentImage->getNumFramesBuffered();

		const viewer::FrameStorage frame = *currentImage->getCurrentFrameStorage();
// 		TS_ASSERT(frame != nullptr);

		if (currentImage->getIsAnimated())
		{
			if (frameTimer.getElapsedTime() > frame.frameTime)
			{
// 				TS_PRINTF("Frame time was %llu ms\n", frame.frameTime.getMilliseconds());

				currentImage->advanceToNextFrame();
				frameTimer.restart();
			}
		}

		math::VC2U size = currentImage->getSize();

		float scale = math::min(1.f, (1000.f) / (float)size.y);

		math::VC2U scaledSize(
			(Uint32)(size.x * scale),
			(Uint32)(size.y * scale)
		);

		sf::VertexArray va = util::makeQuadVertexArray(size.x, size.y);

		sf::RenderStates states;
		states.texture = frame.texture.get();
		states.shader = &shader;
		
		system::WindowManager &wm = application->getManager<system::WindowManager>();
		math::VC2U windowSize = wm.getSize();

		sf::Transform transform;
		transform.translate(windowSize.x / 2.f - scaledSize.x / 2.f, windowSize.y / 2.f - scaledSize.y / 2.f);
		transform.scale(scale, scale);
		states.transform = transform;

		renderWindow.draw(va, states);
	}

	sf::Text statusText;
	statusText.setOutlineThickness(2.f);
	statusText.setFont(*font->getResource());

	{
		statusText.setString(TS_WFMT("%u / %u\n%s\nFrame %u / %u (%u buffered)",
			viewerStateManager->getCurrentImageIndex() + 1,
			viewerStateManager->getNumImages(),
			file::utils::getBasename(viewerStateManager->getCurrentFilepath()),

			frameIndex + 1, frameIndexMax,
			framesBuffered
			));

		statusText.setPosition(10.f, 50.f);
		statusText.setOutlineThickness(2.f);
		statusText.setScale(0.9f, 0.9f);

		renderWindow.draw(statusText);
	}

	{
		std::wstring stats = imageManager.getStats();
		statusText.setString(stats);

		statusText.setPosition(10.f, 300.f);
		statusText.setOutlineThickness(2.f);
		statusText.setScale(0.7f, 0.7f);

		renderWindow.draw(statusText);
	}


}

TS_END_PACKAGE2()
