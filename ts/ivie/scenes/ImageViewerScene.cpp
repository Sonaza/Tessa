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
#include "ts/tessa/threading/ThreadScheduler.h"

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

// 	shader.loadFromFile("convert.frag", sf::Shader::Fragment);
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

		case sf::Event::MouseMoved:
		{
			math::VC2I mousePosition(event.mouseMove.x, event.mouseMove.y);
			math::VC2I mouseDelta = lastMousePosition - mousePosition;

			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				targetPositionOffset.x = math::clamp(targetPositionOffset.x - (float)mouseDelta.x, -1000.f, 1000.f);
				targetPositionOffset.y = math::clamp(targetPositionOffset.y - (float)mouseDelta.y, -1000.f, 1000.f);
			}

			lastMousePosition = mousePosition;
		}
		break;

		case sf::Event::MouseWheelScrolled:
		{
			float delta = event.mouseWheelScroll.delta;
			targetImageScale += delta * 0.15f * targetImageScale;
			targetImageScale = math::clamp(targetImageScale, 0.5f, 10.f);

// 			targetPositionOffset -= dir / 2.f / m_targetScale;
		}
		break;
	}

	return false;
}

void ImageViewerScene::update(const TimeSpan deltaTime)
{
	imageScale += (targetImageScale - imageScale) * deltaTime.getSecondsAsFloat() * 15.f;

	positionOffset += (targetPositionOffset - positionOffset) * deltaTime.getSecondsAsFloat() * 20.f;
}

void ImageViewerScene::imageChanged(SizeType statusText)
{
	targetImageScale = 1.f;
	targetPositionOffset = math::VC2::zero;

	frameTimer.restart();
}

void ImageViewerScene::render(sf::RenderWindow &renderWindow)
{
	renderWindow.clear(sf::Color(30, 30, 30));

	viewer::ImageManager &imageManager = getGigaton<viewer::ImageManager>();

	system::WindowManager &wm = application->getManager<system::WindowManager>();
	math::VC2U windowSize = wm.getSize();

	SizeType frameIndex = 0;
	SizeType frameIndexMax = 1;
	SizeType framesBuffered = 0;

	viewer::Image *currentImage = imageManager.getCurrentImage();
	if (currentImage != nullptr)
	{
		if (currentImage->isDisplayable())
		{
			frameIndex = currentImage->getCurrentFrameIndex();
			frameIndexMax = currentImage->getNumFramesTotal();
			framesBuffered = currentImage->getNumFramesBuffered();

			const viewer::FrameStorage frame = *currentImage->getCurrentFrameStorage();
			if (currentImage->getIsAnimated())
			{
				if (frameTimer.getElapsedTime() > frame.frameTime)
				{
					currentImage->advanceToNextFrame();
					frameTimer.restart();
				}
			}

			math::VC2U size = currentImage->getSize();

			float scale = math::min(1.f, (windowSize.y - 30.f)  / (float)size.y) * imageScale;

			math::VC2U scaledSize(
				(Uint32)(size.x * scale),
				(Uint32)(size.y * scale)
			);

			sf::VertexArray va = util::makeQuadVertexArray(size.x, size.y);

			sf::RenderStates states;
			states.texture = frame.texture.get();

			currentImage->getDisplayShader()->setUniform("u_useAlphaChecker", true);
			states.shader = currentImage->getDisplayShader().get();

			sf::Transform transform;
			transform.translate(
				windowSize.x / 2.f - scaledSize.x / 2.f + positionOffset.x,
				windowSize.y / 2.f - scaledSize.y / 2.f + positionOffset.y
			);
			transform.scale(scale, scale);
			states.transform = transform;

			renderWindow.draw(va, states);
		}

		SharedPointer<sf::Texture> thumbnail = currentImage->getThumbnail();
		if (thumbnail != nullptr)
		{
			sf::Sprite asd;
			asd.setTexture(*thumbnail);
			asd.setPosition(windowSize.x - 180.f, 30.f);
			renderWindow.draw(asd);
		}
	}

	sf::Text statusText;
	statusText.setOutlineThickness(2.f);
	statusText.setFont(*font->getResource());

	{
		statusText.setString(TS_WFMT("%u / %u\n%s",//\nFrame %u / %u (%u buffered)",
			viewerStateManager->getCurrentImageIndex() + 1,
			viewerStateManager->getNumImages(),
			file::utils::getBasename(viewerStateManager->getCurrentFilepath())

			/*, frameIndex + 1, frameIndexMax,
			framesBuffered*/
			));

		statusText.setPosition(10.f, 50.f);
		statusText.setOutlineThickness(2.f);
		statusText.setScale(0.9f, 0.9f);

		renderWindow.draw(statusText);
	}

// 	{
// 		std::wstring stats = imageManager.getStats();
// 		statusText.setString(stats);
// 
// 		statusText.setPosition(10.f, 300.f);
// 		statusText.setOutlineThickness(2.f);
// 		statusText.setScale(0.7f, 0.7f);
// 
// 		renderWindow.draw(statusText);
// 	}

// 	{
// 		threading::ThreadScheduler &ts = getGigaton<threading::ThreadScheduler>();
// 
// 		threading::ThreadScheduler::SchedulerStats stats = ts.getStats();
// 
// 		statusText.setString(TS_FMT(
// 			"Scheduler: %u / %u working  %u pending [%u interval]",
// 			stats.numWorkedTasks, stats.numBackgroundWorkers,
// 			stats.numQueuedTasks, stats.numIntervalTasks
// 		));
// 
// 		statusText.setPosition(10.f, 240.f);
// 		statusText.setOutlineThickness(2.f);
// 		statusText.setScale(0.7f, 0.7f);
// 
// 		renderWindow.draw(statusText);
// 	}
}

TS_END_PACKAGE2()
