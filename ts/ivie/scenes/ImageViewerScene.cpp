#include "Precompiled.h"
#include "ts/ivie/scenes/ImageViewerScene.h"

#include "ts/tessa/file/FileUtils.h"

#include "FreeImage.h"

#include "ts/ivie/util/NaturalSort.h"
#include "ts/ivie/viewer/BackgroundFileScanner.h"
#include "ts/ivie/viewer/ViewerStateManager.h"

#include "ts/ivie/viewer/image/ImageManager.h"
#include "ts/ivie/viewer/image/Image.h"

#include "ts/ivie/util/RenderUtil.h"
#include "ts/tessa/thread/ThreadScheduler.h"

#include "ts/tessa/system/WindowViewManager.h"

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
	windowManager = &getGigaton<system::WindowManager>();
	
	screenResizedBind.connect(windowManager->screenSizeChangedSignal, &ThisClass::screenResized, this);
	
	viewerStateManager = &getGigaton<viewer::ViewerStateManager>();

	imageChangedBind.connect(
		viewerStateManager->currentImageChangedSignal,
		lang::SignalPriority_Normal,
		&ThisClass::imageChanged, this);

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
			switch (event.key.code)
			{
				case sf::Keyboard::Left:
				{
					viewerStateManager->previousImage();
					return true;
				}
				case sf::Keyboard::Right:
				{
					viewerStateManager->nextImage();
					return true;
				}

				case sf::Keyboard::F1:
				{
					showManagerStatus = !showManagerStatus;
					return true;
				}

				case sf::Keyboard::F2:
				{
					showSchedulerStatus = !showSchedulerStatus;
					return true;
				}

				default: /* bop */ break;
			}
		}
		break;

		case sf::Event::MouseButtonPressed:
		{
			switch (event.mouseButton.button)
			{
				case sf::Mouse::Left:
				{
					clickTimer.restart();
					dragged = 0.f;
				}
				return true;

				case sf::Mouse::Right:
				{
					targetImageScale = 1.f;
					targetPositionOffset = math::VC2::zero;
				}
				return true;

				case sf::Mouse::XButton1:
				{
					viewerStateManager->previousImage();
				}
				return true;

				case sf::Mouse::XButton2:
				{
					viewerStateManager->nextImage();
				}
				return true;

				default: break;
			}
		}
		break;

		case sf::Event::MouseButtonReleased:
		{
			switch (event.mouseButton.button)
			{
				case sf::Mouse::Left:
				{
					if (clickTimer.getElapsedTime() < 150_ms && dragged <= 10.f)
					{
						targetImageScale = 1.f / defaultScale;
					}
				}
				return true;

				default: break;
			}

			
		}
		break;

		case sf::Event::MouseMoved:
		{
			math::VC2I mousePosition(event.mouseMove.x, event.mouseMove.y);
			math::VC2 mouseDelta = static_cast<math::VC2>(lastMousePosition - mousePosition);

			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				dragged += mouseDelta.length();

				targetPositionOffset.x = math::clamp(targetPositionOffset.x - mouseDelta.x, -1000.f, 1000.f);
				targetPositionOffset.y = math::clamp(targetPositionOffset.y - mouseDelta.y, -1000.f, 1000.f);

				const system::WindowView &view = windowManager->getApplicationView();

				float scale = defaultScale * imageScale;
				math::VC2 scaledSize = static_cast<math::VC2>(imageSize) * scale;

				if (scaledSize.x > view.size.x)
				{
// 					targetPositionOffset.x framePadding
				}
				else
				{
					targetPositionOffset.x = 0.f;
				}

				if (scaledSize.y > view.size.y)
				{
// 					targetPositionOffset.x 
				}
				else
				{
					targetPositionOffset.y = 0.f;
				}

// 				sf::FloatRect bounds(
// 					targetPositionOffset / -2.f,
// 					scaledSize
// 				);

// 				math::VC2 minClamp = 
			}
			else if (sf::Mouse::isButtonPressed(sf::Mouse::Middle))
			{
				// Scales image by holding down middle mouse: moving mouse pointer up or to the right enlarges, and vice versa
				int32 deltaSign = math::abs(mouseDelta.x) > math::abs(mouseDelta.y) ? -math::sign(mouseDelta.x) : math::sign(mouseDelta.y);
				float delta = mouseDelta.length() * deltaSign;
				targetImageScale += (delta / 140.f) * targetImageScale;
				targetImageScale = math::clamp(targetImageScale, 0.9f, 10.f);
			}

			lastMousePosition = mousePosition;
		}
		break;

		case sf::Event::MouseWheelScrolled:
		{
			float delta = event.mouseWheelScroll.delta;
			targetImageScale += delta * 0.15f * targetImageScale;
			targetImageScale = math::clamp(targetImageScale, 0.9f, 10.f);

// 			targetPositionOffset -= dir / 2.f / m_targetScale;
		}
		break;
	}

	return false;
}

void ImageViewerScene::update(const TimeSpan deltaTime)
{
	const system::WindowView &view = windowManager->getApplicationView();

	framePadding = math::max(20.f, view.size.x * 0.02f);

	defaultScale += (targetDefaultScale - defaultScale) * deltaTime.getSecondsAsFloat() * 8.f;

	imageScale += (targetImageScale - imageScale) * deltaTime.getSecondsAsFloat() * 15.f;
	positionOffset += (targetPositionOffset - positionOffset) * deltaTime.getSecondsAsFloat() * 32.f;

	if (updateImageInfo)
	{
		viewer::ImageManager &imageManager = getGigaton<viewer::ImageManager>();
		viewer::Image *currentImage = imageManager.getCurrentImage();
		if (currentImage != nullptr)
		{
			if (currentImage->isDisplayable())
			{
				imageSize = currentImage->getSize();
				if (imageSize.x > 0 && imageSize.y > 0)
				{
					targetDefaultScale = math::min(
						math::min(1.f, (view.size.x - framePadding) / (float)imageSize.x),
						math::min(1.f, (view.size.y - framePadding) / (float)imageSize.y)
					);
				}
				else
				{
					targetDefaultScale = 1.f;
				}

				defaultScale = targetDefaultScale;

				updateImageInfo = false;
			}
			else if (currentImage->hasError())
			{
				defaultScale = 1.f;
				targetDefaultScale = 1.f;
				updateImageInfo = false;
			}
		}
	}
}

void ImageViewerScene::imageChanged(SizeType statusText)
{
	targetImageScale = 1.f;
	targetPositionOffset = math::VC2::zero;

	frameTimer.restart();

	updateImageInfo = true;
}

void ImageViewerScene::screenResized(const math::VC2U &size)
{
	const system::WindowView &view = windowManager->getApplicationView();

	targetDefaultScale = math::min(
		math::min(1.f, (view.size.x - framePadding) / (float)imageSize.x),
		math::min(1.f, (view.size.y - framePadding) / (float)imageSize.y)
	);
}

void ImageViewerScene::renderApplication(sf::RenderTarget &renderTarget, const system::WindowView &view)
{
	renderTarget.clear(sf::Color(30, 30, 30));

	viewer::ImageManager &imageManager = getGigaton<viewer::ImageManager>();
	viewer::Image *currentImage = imageManager.getCurrentImage();
	if (currentImage != nullptr)
	{
		math::VC2U size = currentImage->getSize();
		float scale = defaultScale * imageScale;

		math::VC2 scaledSize = static_cast<math::VC2>(size) * scale;

		if (currentImage->isDisplayable() && !sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			const viewer::FrameStorage frame = *currentImage->getCurrentFrameStorage();
			if (currentImage->getIsAnimated())
			{
				if (frameTimer.getElapsedTime() > frame.frameTime)
				{
					currentImage->advanceToNextFrame();
					frameTimer.restart();
				}
			}

			sf::VertexArray va = util::makeQuadVertexArrayScaledShadow(
				size.x, size.y, size.x, size.y, 3, sf::Color(0, 0, 0, 170));

			sf::RenderStates states;
			states.texture = frame.texture.get();

			sf::Transform transform;
			transform.translate(scaledSize / -2.f + positionOffset);
			transform.scale(scale, scale);
			states.transform = transform;

			SharedPointer<sf::Shader> shader = currentImage->getDisplayShader(scaledSize, true);
// 			shader->setUniform("u_textureApparentSize", scaledSize);
// 			shader->setUniform("u_useAlphaChecker", true);
			states.shader = shader.get();

			renderTarget.draw(va, states);
		}
		else
		{
			SharedPointer<sf::Texture> thumbnail = currentImage->getThumbnail();
			if (thumbnail != nullptr)
			{
				math::VC2U thumbnailSize = thumbnail->getSize();

				sf::VertexArray va = util::makeQuadVertexArrayScaledShadow(
					size.x, size.y,
					thumbnailSize.x, thumbnailSize.y,
					3, sf::Color(0, 0, 0, 170));

				sf::RenderStates states;
				states.texture = thumbnail.get();

				sf::Transform transform;
				transform.translate(scaledSize / -2.f + positionOffset);
				transform.scale(scale, scale);
				states.transform = transform;

				renderTarget.draw(va, states);
			}
		}
	}

// 	sf::CircleShape shape(20.f);
// 	shape.setOrigin(10.f, 10.f);
// 
// 	{
// 		shape.setFillColor(sf::Color::Red);
// 		shape.setPosition(0.f, 0.f);
// 		renderTarget.draw(shape);
// 	}
// 
// 	{
// 		shape.setFillColor(sf::Color::Green);
// 		shape.setPosition((float)windowSize.x, 0.f);
// 		renderTarget.draw(shape);
// 	}
// 	{
// 		shape.setFillColor(sf::Color::Blue);
// 		shape.setPosition(0.f, (float)windowSize.y);
// 		renderTarget.draw(shape);
// 	}
}

void ImageViewerScene::renderInterface(sf::RenderTarget &renderTarget, const system::WindowView &view)
{
	bool hasError = false;

	viewer::ImageManager &imageManager = getGigaton<viewer::ImageManager>();
	viewer::Image *currentImage = imageManager.getCurrentImage();
	if (currentImage != nullptr)
	{
		SharedPointer<sf::Texture> thumbnail = currentImage->getThumbnail();
		if (thumbnail != nullptr)
		{
			sf::Sprite asd;
			asd.setTexture(*thumbnail);
			asd.setPosition(view.size.x - 180.f, 30.f);
			asd.setScale(0.5f, 0.5f);
			renderTarget.draw(asd);
		}

		hasError = currentImage->hasError();

		if (!hasError && (!currentImage->isDisplayable() || sf::Keyboard::isKeyPressed(sf::Keyboard::Space)))
		{
			sf::Text loadingText;
			loadingText.setOutlineThickness(2.f);
			loadingText.setFont(*font->getResource());

			loadingText.setString("Loading...");

			sf::FloatRect bounds = loadingText.getLocalBounds();
			loadingText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
			loadingText.setPosition(view.size / 2.f);

			renderTarget.draw(loadingText);
		}
	}
	else
	{
		hasError = true;
	}
	
	if (hasError)
	{
		sf::Text errorText;
		errorText.setOutlineThickness(2.f);
		errorText.setFont(*font->getResource());

		if (currentImage != nullptr)
		{
			errorText.setString(TS_FMT(
				"Error: %s", currentImage->getErrorText()
			));
		}
		else
		{
			errorText.setString("Image is null.");
		}

		sf::FloatRect bounds = errorText.getLocalBounds();
		errorText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
		errorText.setPosition(view.size / 2.f);

		renderTarget.draw(errorText);
	}

	sf::Text statusText;
	statusText.setOutlineThickness(2.f);
	statusText.setFont(*font->getResource());

	{
		statusText.setString(TS_WFMT("%u / %u\n%s",
			viewerStateManager->getCurrentImageIndex() + 1,
			viewerStateManager->getNumImages(),
			file::getBasename(viewerStateManager->getCurrentFilepath())
		));

		statusText.setPosition(10.f, 50.f);
		statusText.setOutlineThickness(2.f);
		statusText.setScale(0.9f, 0.9f);

		renderTarget.draw(statusText);
	}

	if (showManagerStatus)
	{
		std::wstring stats = imageManager.getStats();
		statusText.setString(stats);

		statusText.setPosition(10.f, 300.f);
		statusText.setOutlineThickness(2.f);
		statusText.setScale(0.7f, 0.7f);

		renderTarget.draw(statusText);
	}

	if (showSchedulerStatus)
	{
		thread::ThreadScheduler &ts = getGigaton<thread::ThreadScheduler>();

		thread::ThreadScheduler::SchedulerStats stats = ts.getStats();

		statusText.setString(TS_FMT(
			"Scheduler: %u / %u working  %u pending [%u interval]",
			stats.numWorkedTasks, stats.numBackgroundWorkers,
			stats.numQueuedTasks, stats.numIntervalTasks
		));

		statusText.setPosition(10.f, 200.f);
		statusText.setOutlineThickness(2.f);
		statusText.setScale(0.7f, 0.7f);

		renderTarget.draw(statusText);
	}
}

TS_END_PACKAGE2()
