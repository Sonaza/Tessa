#include "Precompiled.h"
#include "ts/ivie/scenes/ImageViewerScene.h"

#include "ts/tessa/file/FileUtils.h"

#include "FreeImage.h"

#include "ts/ivie/util/NaturalSort.h"
#include "ts/ivie/viewer/ViewerManager.h"

#include "ts/ivie/viewer/image/Image.h"

#include "ts/ivie/util/RenderUtil.h"
#include "ts/tessa/thread/ThreadScheduler.h"

#include "ts/tessa/system/WindowViewManager.h"

#include "ts/tessa/profiling/ZoneProfiler.h"

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
	TS_ZONE();

	windowManager = &getGigaton<system::WindowManager>();
	
	filesDroppedBind.connect(windowManager->filesDroppedSignal, &ThisClass::filesDropped, this);
	screenResizedBind.connect(windowManager->screenSizeChangedSignal, &ThisClass::screenResized, this);
	
	viewerManager = &getGigaton<viewer::ViewerManager>();

	filelistChangedBind.connect(
		viewerManager->filelistChangedSignal,
		lang::SignalPriority_Normal,
		&ThisClass::filelistChanged, this);

	imageChangedBind.connect(
		viewerManager->currentImageChangedSignal,
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
	static std::vector<float> presetScales = {
		0.167f,
		0.25f,
		0.333f,
		0.5f,
		0.667f,
		1.f,
		1.5f,
		2.f,
	};

	const bool ctrlDown = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);

	switch (event.type)
	{
		case sf::Event::KeyPressed:
		{
			switch (event.key.code)
			{
				case sf::Keyboard::Left:
				case sf::Keyboard::Right:
				{
					if (changeTimer.getElapsedTime() >= 150_ms)
					{
						int32 diff = (event.key.code == sf::Keyboard::Left ? -1 : 1) * (ctrlDown ? 10 : 1);
						viewerManager->changeImage(diff);
						changeTimer.restart();
					}
					return true;
				}
				case sf::Keyboard::PageUp:
				case sf::Keyboard::PageDown:
				{
					if (changeTimer.getElapsedTime() >= 150_ms)
					{
						int32 diff = (event.key.code == sf::Keyboard::PageUp ? -10 : 10);
						viewerManager->changeImage(diff);
						changeTimer.restart();
					}
					return true;
				}

				case sf::Keyboard::Add:
				case sf::Keyboard::Subtract:
				{
					int32 dir = (event.key.code == sf::Keyboard::Subtract ? -1 : 1);

// 					if (!ctrlDown)
// 					{
						targetImageScale += dir * 0.15f * targetImageScale;
// 					}
// 					else
// 					{
// 						float currentScale = defaultScale * targetImageScale;
// 						
// 						float scale = -1.f;
// 						for (int32 i = 0; i < presetScales.size(); ++i)
// 						{
// 							if (dir > 0)
// 							{
// 								if (presetScales[i] > currentScale)
// 								{
// 									scale = presetScales[i];
// 									break;
// 								}
// 
// 							}
// 							else
// 							{
// 
// 							}
// 						}
// 
// 						if (scale > 0.f)
// 						{
// 
// 						}
// 					}

					targetImageScale = math::clamp(targetImageScale, 0.9f, 10.f);

					return true;
				}

				case sf::Keyboard::S:
				{
					displaySmooth = !displaySmooth;
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

				case sf::Keyboard::F5:
				{
					if (currentImage != nullptr)
						currentImage->reload();

					return true;
				}

				case sf::Keyboard::M:
				{
					displayMode = displayMode == Normal ? Manga : Normal;
					switch (displayMode)
					{
						case Normal:
						{
							/* blop */
						}
						break;

						case Manga:
						{
							targetImageScale = targetImageScale = 1.f / defaultScale;
							targetPositionOffset.x = 0.f;
							targetPositionOffset.y = 10000.f;
							enforceOversizeLimits(defaultScale * targetImageScale);
						}
						break;
					}
					return true;
				}

				case sf::Keyboard::R:
				{
					recursiveFileSearch = !recursiveFileSearch;
					viewerManager->setRecursiveScan(recursiveFileSearch);
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
					if (displayMode == Normal)
					{
						clickTimer.restart();
						dragged = 0.f;
					}
				}
				return true;

				case sf::Mouse::Right:
				{
					if (displayMode == Normal)
					{
						targetImageScale = 1.f;
						targetPositionOffset = math::VC2::zero;
					}
				}
				return true;

				case sf::Mouse::XButton1:
				case sf::Mouse::XButton2:
				{
					int32 diff = (event.mouseButton.button == sf::Mouse::XButton1 ? -1 : 1) * (ctrlDown ? 10 : 1);
					viewerManager->changeImage(diff);
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
			math::VC2 mouseDelta = static_cast<math::VC2>(mousePosition - lastMousePosition);

			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				dragged += mouseDelta.length();
				targetPositionOffset += mouseDelta;
				enforceOversizeLimits(defaultScale * imageScale);
			}
			
			if (sf::Mouse::isButtonPressed(sf::Mouse::Middle))
			{
				// Scales image by holding down middle mouse: moving mouse pointer up or to the right enlarges, and vice versa
				int32 deltaSign = math::abs(mouseDelta.x) > math::abs(mouseDelta.y) ? math::sign(mouseDelta.x) : -math::sign(mouseDelta.y);
				float delta = mouseDelta.length() * deltaSign;
				targetImageScale += (delta / 140.f) * targetImageScale;
				targetImageScale = math::clamp(targetImageScale, 0.9f, 10.f);
				enforceOversizeLimits(defaultScale * targetImageScale);
			}

			lastMousePosition = mousePosition;
		}
		break;

		case sf::Event::MouseWheelScrolled:
		{
			float delta = event.mouseWheelScroll.delta;

			switch (displayMode)
			{
				case Normal:
				{
					targetImageScale += delta * 0.15f * targetImageScale;
					targetImageScale = math::clamp(targetImageScale, 0.9f, 10.f);

					if (delta > 0.f)
					{
						const system::WindowView &view = windowManager->getApplicationView();

						math::VC2 mouse(
							(float)event.mouseWheelScroll.x,
							(float)event.mouseWheelScroll.y
						);

// 						sf::Transform tf;
// 						tf.translate(positionOffset).scale(targetImageScale, targetImageScale);
// 						math::VC2 mousetf = tf.getInverse().transformPoint(mouse);
// 						TS_PRINTF("mouse %s\n", mousetf.toString());

						math::VC2 center = view.size / 2.f;
						math::VC2 diff = mouse - center;

						math::VC2 offset = diff;

						targetPositionOffset -= offset;
						enforceOversizeLimits(defaultScale * targetImageScale);

//		 				TS_PRINTF("mouse %s, center %s, diff %s\n", mouse.toString(), center.toString(), diff.toString());
// 						TS_PRINTF("diff %s\n", offset.toString());
					}
				}
				break;

				case Manga:
				{
					targetPositionOffset.y += delta * 140.f;
					enforceOversizeLimits(defaultScale * targetImageScale);
				}
				break;
			}
			
			

		}
		break;
	}

	return false;
}

bool ImageViewerScene::updateImageInfo()
{
	TS_ZONE();

	if (currentImage == nullptr)
		return true;
	
	const system::WindowView &view = windowManager->getApplicationView();

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

		return true;
	}
	else if (currentImage->hasError())
	{
		defaultScale = 1.f;
		targetDefaultScale = 1.f;
		return true;
	}

	return false;
}

void ImageViewerScene::update(const TimeSpan deltaTime)
{
	TS_ZONE();

	const system::WindowView &view = windowManager->getApplicationView();

	framePadding = math::max(20.f, view.size.x * 0.02f);

	if (pendingImageInfo && updateImageInfo())
		pendingImageInfo = false;

	if (!sf::Mouse::isButtonPressed(sf::Mouse::Middle))
	{
		targetImageScale = math::max(1.f, targetImageScale);
	}

	defaultScale += (targetDefaultScale - defaultScale) * deltaTime.getSecondsAsFloat() * 8.f;
	imageScale += (targetImageScale - imageScale) * deltaTime.getSecondsAsFloat() * 25.f;

	positionOffset += (targetPositionOffset - positionOffset) * deltaTime.getSecondsAsFloat() * 32.f;

	{
		float scale = defaultScale * imageScale;
		math::VC2 scaledSize = static_cast<math::VC2>(imageSize) * scale;

		math::VC2 oversize = (scaledSize - view.size) / 2.f;
		oversize.x = math::max(0.f, oversize.x);
		oversize.y = math::max(0.f, oversize.y);

		positionOffset.x = math::clamp(positionOffset.x, -oversize.x, oversize.x);
		positionOffset.y = math::clamp(positionOffset.y, -oversize.y, oversize.y);
	}
}

void ImageViewerScene::enforceOversizeLimits(float scale)
{
	const system::WindowView &view = windowManager->getApplicationView();

// 	float scale = defaultScale * imageScale;
	math::VC2 scaledSize = static_cast<math::VC2>(imageSize) * scale;

	math::VC2 oversize = (scaledSize - view.size) / 2.f;
	oversize.x = math::max(0.f, oversize.x);
	oversize.y = math::max(0.f, oversize.y);

	positionOversizeLimit = oversize;

	targetPositionOffset.x = math::clamp(targetPositionOffset.x, -oversize.x, oversize.x);
	targetPositionOffset.y = math::clamp(targetPositionOffset.y, -oversize.y, oversize.y);

	positionOffset.x = math::clamp(positionOffset.x, -oversize.x, oversize.x);
	positionOffset.y = math::clamp(positionOffset.y, -oversize.y, oversize.y);
}

void ImageViewerScene::imageChanged(SharedPointer<viewer::Image> image)
{
	TS_ZONE();

	currentImage = image;
	if (updateImageInfo() == false)
		pendingImageInfo = true;

	switch (displayMode)
	{
		case Normal:
		{
			targetImageScale = 1.f;
			targetPositionOffset = math::VC2::zero;
		}
		break;

		case Manga:
		{
// 			targetImageScale = 1.f;
			targetPositionOffset.x = 0.f;
			targetPositionOffset.y = 10000.f;
			enforceOversizeLimits(defaultScale * targetImageScale);
		}
		break;
	}

	frameTimer.restart();
}

void ImageViewerScene::filelistChanged(SizeType numFiles)
{
	if (numFiles == 0)
		currentImage = nullptr;
}

void ImageViewerScene::screenResized(const math::VC2U &size)
{
	const system::WindowView &view = windowManager->getApplicationView();

	targetDefaultScale = math::min(
		math::min(1.f, (view.size.x - framePadding) / (float)imageSize.x),
		math::min(1.f, (view.size.y - framePadding) / (float)imageSize.y)
	);
}

void ImageViewerScene::filesDropped(const std::vector<system::DroppedFile> &files)
{
	if (files.empty())
		return;

	viewerManager->setFilepath(files[0].filepath);
}

void ImageViewerScene::renderApplication(sf::RenderTarget &renderTarget, const system::WindowView &view)
{
	TS_ZONE();

	renderTarget.clear(sf::Color(30, 30, 30));

	if (currentImage != nullptr)
	{
		math::VC2U size = currentImage->getSize();
		float scale = defaultScale * imageScale;

		math::VC2 scaledSize = static_cast<math::VC2>(size) * scale;

		bool displayable = currentImage->isDisplayable() && !pendingImageInfo;
		if (displayable)
		{
			viewer::FrameStorage frame = *currentImage->getCurrentFrameStorage();
			if (frame.texture != nullptr)
			{
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

				frame.texture->setSmooth(displaySmooth);

				sf::RenderStates states;
				states.texture = frame.texture.get();

				sf::Transform transform;
				transform.translate(scaledSize / -2.f + positionOffset);
				transform.scale(scale, scale);
				states.transform = transform;

				states.shader = currentImage->getDisplayShader(scale).get();

				{
					TS_ZONE_NAMED("renderTarget.draw image");
					renderTarget.draw(va, states);
				}
			}
			else
			{
				displayable = false;
			}
		}
		
		if (!displayable)
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

				{
					TS_ZONE_NAMED("renderTarget.draw thumbnail");
					renderTarget.draw(va, states);
				}
			}
		}
	}
}

void ImageViewerScene::renderInterface(sf::RenderTarget &renderTarget, const system::WindowView &view)
{
	TS_ZONE();

	const SizeType numImages = viewerManager->getNumImages();
	bool hasError = false;

	if (currentImage != nullptr)
	{
// 		SharedPointer<sf::Texture> thumbnail = currentImage->getThumbnail();
// 		if (thumbnail != nullptr)
// 		{
// 			sf::Sprite asd;
// 			asd.setTexture(*thumbnail);
// 			asd.setPosition(view.size.x - 180.f, 30.f);
// 			asd.setScale(0.5f, 0.5f);
// 			renderTarget.draw(asd);
// 		}

		hasError = currentImage->hasError();

		if (!hasError && !currentImage->isDisplayable())
		{
			sf::Text loadingText;
// 			loadingText.setOutlineThickness(2.f);
			loadingText.setFont(*font->getResource());

			loadingText.setString("Loading...");

			sf::FloatRect bounds = loadingText.getLocalBounds();
			loadingText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
			loadingText.setPosition(view.size / 2.f);

			{
				TS_ZONE_NAMED("renderTarget.draw loadingText");
				renderTarget.draw(loadingText);
			}
		}
	}
	else
	{
		hasError = true;
	}
	
	sf::Text statusText("", *font->getResource());
	statusText.setOutlineThickness(2.f);

	if (numImages > 0)
	{
		{
			TS_ZONE_NAMED("statusText 1");

			{
				TS_ZONE_NAMED("statusText 1 sprintf");
				statusText.setString(TS_WFMT("%u / %u",
					viewerManager->getCurrentImageIndex() + 1,
					viewerManager->getNumImages()
				));
			}

			statusText.setPosition(10.f, 35.f);
			statusText.setScale(0.9f, 0.9f);

			{
				TS_ZONE_NAMED("statusText 1 draw");
				renderTarget.draw(statusText);
			}

			if (viewerManager->isScanningFiles() && !viewerManager->isFirstScanComplete())
			{
				math::FloatRect bounds = statusText.getGlobalBounds();

				sf::CircleShape c(5.f);

				math::VC2 position(bounds.maxbounds.x + 10.f, bounds.getCenter().y);

				float t = elapsedTimer.getElapsedTime().getSecondsAsFloat();
				position.x += 20.f + std::cos(t * 2.f) * 20.f;
				position.y += - math::abs(std::cos(t * 10.f) * 10.f) * ((std::cos(t * 5.f) + 1.f) * 0.5f);

				c.setPosition(position);
				c.setFillColor(sf::Color::White);
				c.setOutlineColor(sf::Color::Black);

				renderTarget.draw(c);
			}
		}

		{
			TS_ZONE_NAMED("statusText 2");

			String filename = file::getBasename(viewerManager->getCurrentFilepath());

			statusText.setString(
				filename
			);

			statusText.setPosition(10.f, 75.f);

			float filenameScale = math::clamp(60.f / (float)filename.getSize(), 0.5f, 1.f);
			statusText.setScale(0.9f * filenameScale, 0.9f * filenameScale);

			renderTarget.draw(statusText);
		}
	}

	if (hasError)
	{
		TS_ZONE_NAMED("errortext 1");

		sf::Text errorText("", *font->getResource());
// 		errorText.setOutlineThickness(2.f);

		if (currentImage != nullptr)
		{
			const String &errorStr = currentImage->getErrorText();
			errorText.setString(TS_WFMT(
				"Error: %s", (!errorStr.isEmpty() ? errorStr : "error has error.")
			));
		}
		else if (numImages == 0)
		{
			errorText.setString("Directory has no displayable files.");
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
	else
	{
		{
			TS_ZONE_NAMED("statusText 3");

			float scale = defaultScale * imageScale;

			if (math::abs(scale - 1.f) < 0.001f)
			{
				statusText.setString(TS_FMT("%.1f%%\n%u x %u",
					scale * 100.f,
					imageSize.x, imageSize.y
				));
			}
			else
			{
				math::VC2U scaledSize = static_cast<math::VC2U>(
					static_cast<math::VC2>(imageSize) * scale + math::VC2(0.5f, 0.5f));

				statusText.setString(TS_FMT("%.1f%%\n%u x %u (%u x %u)",
					scale * 100.f,
					imageSize.x, imageSize.y,
					scaledSize.x, scaledSize.y
				));
			}

			statusText.setPosition(10.f, 115.f);
			statusText.setScale(0.7f, 0.7f);

			renderTarget.draw(statusText);
		}
	}

	if (showManagerStatus || showSchedulerStatus)
	{
		TS_ZONE_NAMED("debugText 2");

		sf::Text debugText;
// 		debugText.setOutlineThickness(2.f);
		debugText.setFont(*font->getResource());

		if (showManagerStatus)
		{
			std::wstring stats = viewerManager->getStats();
			debugText.setString(stats);

			debugText.setPosition(10.f, 300.f);
			debugText.setScale(0.5f, 0.5f);

			renderTarget.draw(debugText);
		}

		if (showSchedulerStatus)
		{
			thread::ThreadScheduler &ts = getGigaton<thread::ThreadScheduler>();

			thread::ThreadScheduler::SchedulerStats stats = ts.getStats();

			debugText.setString(TS_FMT(
				"Scheduler: %u / %u working  %u pending [%u interval]",
				stats.numWorkedTasks, stats.numBackgroundWorkers,
				stats.numQueuedTasks, stats.numIntervalTasks
			));

			debugText.setPosition(10.f, 200.f);
			debugText.setScale(0.7f, 0.7f);

			renderTarget.draw(debugText);
		}
	}
}

TS_END_PACKAGE2()
