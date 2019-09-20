#include "Precompiled.h"
#include "ts/ivie/scenes/ImageViewerScene.h"

#include "ts/tessa/file/FileUtils.h"

#include "FreeImage.h"

#include "ts/ivie/util/NaturalSort.h"
#include "ts/ivie/viewer/ViewerManager.h"

#include "ts/ivie/util/RenderUtil.h"
#include "ts/tessa/thread/ThreadScheduler.h"

#include "ts/tessa/system/WindowViewManager.h"

#include "ts/tessa/profiling/ZoneProfiler.h"

#if TS_PLATFORM == TS_WINDOWS
#include "ts/tessa/common/WindowsUtils.h"
#endif

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
		viewerManager->imageChangedSignal,
		lang::SignalPriority_Normal,
		&ThisClass::imageChanged, this);

	return true;
}

void ImageViewerScene::stop()
{
}

void ImageViewerScene::loadResources(resource::ResourceManager &rm)
{
	font = rm.loadFont("viewer_font", "SourceHanSans-Medium.ttc", true);
	backgroundShader = rm.loadShader("background_shader", "shader/background_gradient.frag", true);
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
					SizeType option = (SizeType)viewerManager->getSorting();
					option = (option + 1) % viewer::SortingStyle_NumOptions;
					viewerManager->setSorting((viewer::SortingStyle)option);
					return true;
				}

				case sf::Keyboard::P:
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
					if (current.image != nullptr)
						current.image->reload();

					return true;
				}

				case sf::Keyboard::F6:
				{
					backgroundShader->reloadResource();
					return true;
				}

				case sf::Keyboard::G:
				{
#if TS_PLATFORM == TS_WINDOWS
					windows::openExplorerToFile(viewerManager->getCurrentFilepath());
					return true;
#endif
				}
				break;

				case sf::Keyboard::D:
				{
#if TS_PLATFORM == TS_WINDOWS
					windows::openFileWithDialog(viewerManager->getCurrentFilepath());
					return true;
#endif
				}
				break;

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
					viewerManager->setRecursiveScan(!viewerManager->getIsRecursiveScan());
					return true;
				}

				case sf::Keyboard::H:
				{
					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
					{
						viewerInfoMode = viewerInfoMode == ViewerInfo_DisplayAll ? ViewerInfo_HideAll : ViewerInfo_DisplayAll; 
					}
					else
					{
						viewerInfoMode = viewerInfoMode != ViewerInfo_IndexOnly ? ViewerInfo_IndexOnly : ViewerInfo_DisplayAll;
					}
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
					if (clickTimer.getElapsedTime() < 250_ms && dragged <= 10.f)
					{
						TS_ASSERT(defaultScale > 0.f);
						targetImageScale = 1.f / defaultScale;

						{
							math::VC2 mouse(
								(float)event.mouseButton.x,
								(float)event.mouseButton.y
							);

							const system::WindowView &view = windowManager->getApplicationView();

							float currentScale = defaultScale * imageScale;
							math::VC2 diff = calculateMouseDiff(view, mouse, currentScale, 1.f);

							float distanceFromEdgeX = math::abs(mouse.x - (view.size.x / 2.f)) / (view.size.x / 2.f);
							float distanceFromEdgeY = math::abs(mouse.y - (view.size.y / 2.f)) / (view.size.y / 2.f);
							float distanceFromEdge = math::max(distanceFromEdgeX, distanceFromEdgeY);
							float multiplier = (float)std::pow((distanceFromEdge - 0.5f) * 2.f, 6) * 0.4f + 1.f;

							targetPositionOffset = positionOffset + diff * multiplier;

							enforceOversizeLimits(defaultScale * math::max(targetImageScale, imageScale));
						}
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
				enforceOversizeLimits(defaultScale * imageScale);
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
					targetImageScale = math::clamp(targetImageScale, 1.f, 10.f);

					{
						math::VC2 mouse(
							(float)event.mouseWheelScroll.x,
							(float)event.mouseWheelScroll.y
						);

						float currentScale = defaultScale * imageScale;
						float targetScale = defaultScale * targetImageScale;

						const system::WindowView &view = windowManager->getApplicationView();
						math::VC2 diff = calculateMouseDiff(view, mouse, currentScale, targetScale);

						float multiplier = 1.f;
						if (delta > 0.f)
						{
							float distanceFromEdgeX = math::abs(mouse.x - (view.size.x / 2.f)) / (view.size.x / 2.f);
							float distanceFromEdgeY = math::abs(mouse.y - (view.size.y / 2.f)) / (view.size.y / 2.f);
							float distanceFromEdge = math::max(distanceFromEdgeX, distanceFromEdgeY);
							multiplier = (float)std::pow((distanceFromEdge - 0.5f) * 2.f, 4) * 0.13f + 1.f;
						}

						targetPositionOffset = positionOffset + diff * multiplier * targetScale;
						
						enforceOversizeLimits(defaultScale * math::max(targetImageScale, imageScale));
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

	TS_ASSERT(current.image != nullptr);

	image::ImageData imageData;
	if (current.image->getImageData(imageData))
	{
		current.data = std::move(imageData);
		current.hasData = true;

		const system::WindowView &view = windowManager->getApplicationView();

		TS_ASSERT(current.data.size.x > 0 && current.data.size.y > 0);

		float previousDefaultScale = defaultScale;

		targetDefaultScale = math::min(
			math::min(1.f, (view.size.x - framePadding) / (float)current.data.size.x),
			math::min(1.f, (view.size.y - framePadding) / (float)current.data.size.y)
		);
		defaultScale = targetDefaultScale;

		switch (displayMode)
		{
		case Normal:
		{
			targetImageScale = 1.f;
			if (defaultScale == 1.f)
				imageScale = targetImageScale;

			targetPositionOffset = math::VC2::zero;
		}
		break;

		case Manga:
		{
			targetPositionOffset.x = 0.f;
			targetPositionOffset.y = 10000.f;
			enforceOversizeLimits(defaultScale * targetImageScale);
		}
		break;
		}

		frameTimer.restart();

		return true;
	}
	else if (current.image->hasError())
	{
		current.hasError = true;

		defaultScale = targetDefaultScale = 1.f;
		imageScale = targetImageScale = 1.f;
		positionOffset = targetPositionOffset = math::VC2::zero;
		return true;
	}
	return false;
}

void ImageViewerScene::update(const TimeSpan deltaTime)
{
	TS_ZONE();

	const system::WindowView &view = windowManager->getApplicationView();

	framePadding = math::max(20.f, view.size.x * 0.02f);

	if (current.image != nullptr && !current.hasError && !current.hasData)
		updateImageInfo();

	if (!sf::Mouse::isButtonPressed(sf::Mouse::Middle))
	{
		targetImageScale = math::max(1.f, targetImageScale);
	}
}

void ImageViewerScene::updateFrequent(const TimeSpan deltaTime)
{
	TS_ZONE();

	const system::WindowView &view = windowManager->getApplicationView();

	float delta = deltaTime.getSecondsAsFloat();

	defaultScale += (targetDefaultScale - defaultScale) * delta * 8.f;

	imageScale += (targetImageScale - imageScale) * delta * 24.f;

	float positionTweenSpeed = math::abs(targetImageScale - imageScale) < 0.01f ? 35.f : 24.f;
	positionOffset += (targetPositionOffset - positionOffset) * delta * positionTweenSpeed;

	{
		float targetIndexAlpha = viewerInfoMode != ViewerInfo_HideAll ? 1.f : 0.f;
		viewerInfoAlpha.index += (targetIndexAlpha - viewerInfoAlpha.index) * delta * 16.f;

		float targetOtherAlpha = viewerInfoMode == ViewerInfo_DisplayAll ? 1.f : 0.f;
		viewerInfoAlpha.other += (targetOtherAlpha - viewerInfoAlpha.other) * delta * 16.f;
	}

	enforceOversizeLimits(defaultScale * imageScale, false);
}

void ImageViewerScene::enforceOversizeLimits(float scale, bool enforceTarget)
{
	if (!current.hasData)
		return;

	const system::WindowView &view = windowManager->getApplicationView();

	math::VC2 scaledSize = static_cast<math::VC2>(current.data.size) * scale;

	math::VC2 oversize = (scaledSize - view.size) / 2.f;
	oversize.x = math::max(0.f, oversize.x);
	oversize.y = math::max(0.f, oversize.y);

	positionOversizeLimit = oversize;

	if (enforceTarget)
	{
		targetPositionOffset.x = math::clamp(targetPositionOffset.x, -oversize.x, oversize.x);
		targetPositionOffset.y = math::clamp(targetPositionOffset.y, -oversize.y, oversize.y);
	}

	positionOffset.x = math::clamp(positionOffset.x, -oversize.x, oversize.x);
	positionOffset.y = math::clamp(positionOffset.y, -oversize.y, oversize.y);
}

math::VC2 ImageViewerScene::calculateMouseDiff(const system::WindowView &view,
	const math::VC2 &mousePos, float currentScale, float targetScale)
{
	if (!current.hasData)
		return math::VC2::zero;

	math::VC2 converted = view.convertToViewCoordinate(mousePos);
	math::VC2 imageCenter = static_cast<math::VC2>(current.data.size) / 2.f;

	math::Transform currentTransform;
	currentTransform
		.translate(positionOffset)
		.scale(currentScale, currentScale)
		.translate(-imageCenter);

	math::Transform targetTransform;
	targetTransform
		.translate(positionOffset)
		.scale(targetScale, targetScale)
		.translate(-imageCenter);

	math::VC2 currentMouseImagePos = currentTransform.getInverse().transformPoint(converted);
	math::VC2 targetMouseImagePos = targetTransform.getInverse().transformPoint(converted);

	return targetMouseImagePos - currentMouseImagePos;
}

void ImageViewerScene::imageChanged(SharedPointer<image::Image> image)
{
	TS_ZONE();

	current = CurrentState();
	current.image = image;

	if (current.image != nullptr)
		updateImageInfo();
}

void ImageViewerScene::filelistChanged(SizeType numFiles)
{
	if (numFiles == 0)
		current.image = nullptr;
}

void ImageViewerScene::screenResized(const math::VC2U &size)
{
	const system::WindowView &view = windowManager->getApplicationView();

	targetDefaultScale = math::min(
		math::min(1.f, (view.size.x - framePadding) / (float)current.data.size.x),
		math::min(1.f, (view.size.y - framePadding) / (float)current.data.size.y)
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

	if (backgroundShader && backgroundShader->isLoaded())
	{
		sf::RectangleShape bg(view.size);
		bg.setFillColor(sf::Color(30, 30, 30));
		bg.setOrigin(view.size / 2.f);

		sf::Shader &bgShader = *backgroundShader->getResource();
		bgShader.setUniform("u_resolution", view.size);

		renderTarget.draw(bg, &bgShader);
	}

	if (current.image != nullptr)
	{
		float scale = defaultScale * imageScale;

		math::VC2 scaledSize = static_cast<math::VC2>(current.data.size) * scale;

		bool displayable = current.image->isDisplayable() && current.hasData;
		if (displayable)
		{
			image::FrameStorage currentFrame = *current.image->getCurrentFrameStorage();
			current.frameTime = currentFrame.frameTime;

			if (currentFrame.texture != nullptr)
			{
				if (current.image->getIsAnimated())
				{
					if (frameTimer.getElapsedTime() > currentFrame.frameTime)
					{
						current.image->advanceToNextFrame();
						frameTimer.restart();

// 						currentFrame = *current.image->getCurrentFrameStorage();
// 						current.frameTime = currentFrame.frameTime;
					}
				}

				sf::VertexArray va = util::makeQuadVertexArrayScaled(
					current.data.size.x, current.data.size.y,
					current.data.size.x, current.data.size.y
				);

				currentFrame.texture->setSmooth(displaySmooth);

				sf::RenderStates states;
				states.texture = currentFrame.texture.get();

				math::Transform transform;
				transform
					.translate(scaledSize / -2.f + positionOffset)
					.scale(scale, scale);
				states.transform = (sf::Transform)transform;

				image::DisplayShaderParams params;
				params.viewSize = view.size;
				params.scale = scale;
				params.offset = positionOffset;
				states.shader = current.image->getDisplayShader(&params);

				renderTarget.draw(va, states);
			}
			else
			{
				displayable = false;
			}
		}
		
		if (!displayable && current.hasData)
		{
			SharedPointer<sf::Texture> thumbnail = current.image->getThumbnail();
			if (thumbnail != nullptr)
			{
				math::VC2U thumbnailSize = thumbnail->getSize();

				sf::VertexArray va = util::makeQuadVertexArrayScaledShadow(
					current.data.size.x, current.data.size.y,
					thumbnailSize.x, thumbnailSize.y,
					3, sf::Color(0, 0, 0, 170));

				sf::RenderStates states;
				states.texture = thumbnail.get();

				math::Transform transform;
				transform
					.translate(scaledSize / -2.f + positionOffset)
					.scale(scale, scale);
				states.transform = (sf::Transform)transform;

				renderTarget.draw(va, states);
			}
		}
	}
}

void ImageViewerScene::renderInterface(sf::RenderTarget &renderTarget, const system::WindowView &view)
{
	TS_ZONE();

	const SizeType numImages = viewerManager->getNumImages();
	bool hasError = false;

	if (current.image != nullptr)
	{
// 		SharedPointer<sf::Texture> thumbnail = current.image->getThumbnail();
// 		if (thumbnail != nullptr)
// 		{
// 			sf::Sprite asd;
// 			asd.setTexture(*thumbnail);
// 			asd.setPosition(view.size.x - 180.f, 30.f);
// 			asd.setScale(0.5f, 0.5f);
// 			renderTarget.draw(asd);
// 		}

		hasError = current.image->hasError();

		if (!hasError && !current.image->isDisplayable())
		{
			sf::Text loadingText;
			loadingText.setOutlineThickness(2.f);
			loadingText.setFont(*font->getResource());

			SizeType numDots = 1 + ((elapsedTimer.getElapsedTime().getMilliseconds() / 900) % 3);
			loadingText.setString(TS_FMT("Loading%s", std::string(numDots, '.')));

			sf::FloatRect bounds = loadingText.getLocalBounds();
			loadingText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
			loadingText.setPosition(view.size / 2.f + math::VC2(0.f, 25.f));

			renderTarget.draw(loadingText);

			drawLoaderGadget(renderTarget, view.size / 2.f + math::VC2(0.f, -25.f), 30.f);
		}
	}
	else
	{
		hasError = true;
	}

	if (viewerInfoAlpha.index >= 0.003f && numImages > 0)
	{
		sf::Text statusText("", *font->getResource());
		statusText.setOutlineThickness(2.f);

		float bottomOffset = (1.f - viewerInfoAlpha.other) * 60.f;
		float textBottomOffset = 27.f - bottomOffset;

		statusText.setFillColor(sf::Color(255, 255, 255, (uint8)(viewerInfoAlpha.index * 255)));
		statusText.setOutlineColor(sf::Color(0, 0, 0, (uint8)(viewerInfoAlpha.index * 255)));

		{
			statusText.setString(TS_WFMT("%u / %u",
				viewerManager->getCurrentImageIndex() + 1,
				viewerManager->getNumImages()
			));

			statusText.setOrigin(0.f, statusText.getLocalBounds().height);

			statusText.setPosition(
				16.f,
				view.size.y - (!hasError ? 60.f : 0.f) - textBottomOffset
			);
			statusText.setScale(0.9f, 0.9f);

			renderTarget.draw(statusText);

			if (viewerManager->isScanningFiles() && !viewerManager->isFirstScanComplete())
			{
				math::FloatRect bounds = statusText.getGlobalBounds();

				math::VC2 position(bounds.maxbounds.x + 30.f, bounds.getCenter().y - 5.f);
				drawLoaderGadget(renderTarget, position);
			}
		}

		if (viewerInfoAlpha.other >= 0.03f)
		{
			statusText.setFillColor(sf::Color(255, 255, 255, (uint8)(viewerInfoAlpha.other * 255)));
			statusText.setOutlineColor(sf::Color(0, 0, 0, (uint8)(viewerInfoAlpha.other * 255)));

			{
				const String filename = viewerManager->getCurrentFilepath(false);
				TS_ASSERT(!filename.isEmpty());

// 				String dirname = file::getDirname(filename);
				String basename = file::getBasename(filename);

				statusText.setString(basename);

				statusText.setOrigin(statusText.getLocalBounds().width, 0.f);

				float filenameScale = math::clamp(60.f / (float)basename.getSize(), 0.5f, 1.f) * 0.9f;
				statusText.setScale(filenameScale, filenameScale);

				statusText.setPosition(
					view.size.x - 16.f,
					view.size.y - 30.f * filenameScale - textBottomOffset
				);

				renderTarget.draw(statusText);
			}

			if (!hasError)
			{
				statusText.setOrigin(0.f, 0.f);

				float scale = defaultScale * imageScale;

				if (math::abs(scale - 1.f) < 0.001f)
				{
					statusText.setString(TS_FMT("%.1f%%\n%u x %u",
						scale * 100.f,
						current.data.size.x, current.data.size.y
					));
				}
				else
				{
					math::VC2U scaledSize = static_cast<math::VC2U>(
						static_cast<math::VC2>(current.data.size) * scale + math::VC2(0.5f, 0.5f));

					statusText.setString(TS_FMT("%.1f%%\n%u x %u (%u x %u)",
						scale * 100.f,
						current.data.size.x, current.data.size.y,
						scaledSize.x, scaledSize.y
					));
				}

				statusText.setPosition(
					16.f,
					view.size.y - 50.f - textBottomOffset);
				statusText.setScale(0.7f, 0.7f);

				renderTarget.draw(statusText);
			}

			if (current.image != nullptr && current.image->getIsAnimated())
			{
				float progress = current.image->getAnimationProgress(frameTimer.getElapsedTime());

				math::VC2 fullsize(view.size.x + 2.f, 4.f);
				float offset = 1.f;

				{
					sf::RectangleShape bar(fullsize);
					bar.setFillColor(sf::Color(0, 0, 0, (uint8)(viewerInfoAlpha.other * 160)));
					bar.setOutlineColor(sf::Color(80, 80, 80, (uint8)(viewerInfoAlpha.other * 200)));
					bar.setOutlineThickness(1.f);

					bar.setOrigin(fullsize.x * 0.5f, fullsize.y);
					bar.setPosition(
						view.size.x / 2.f,
						view.size.y - offset + bottomOffset
					);

					renderTarget.draw(bar);
				}

				{
					sf::RectangleShape bar(math::VC2(fullsize.x * progress, fullsize.y));
					bar.setFillColor(sf::Color(255, 255, 255, (uint8)(viewerInfoAlpha.other * 230)));

					bar.setOrigin(0.f, fullsize.y);
					bar.setPosition(
						view.size.x / 2.f - fullsize.x / 2.f,
						view.size.y - offset + bottomOffset
					);

					renderTarget.draw(bar, sf::BlendAdd);
				}

				{
					sf::CircleShape dot(5.f);
					dot.setFillColor(sf::Color(255, 220, 40, (uint8)(viewerInfoAlpha.other * 255)));
					dot.setOutlineColor(sf::Color(127, 110, 20, (uint8)(viewerInfoAlpha.other * 120)));
					dot.setOutlineThickness(2.f);

					dot.setOrigin(5.f, 5.f);
					dot.setPosition(
						view.size.x / 2.f - fullsize.x / 2.f + fullsize.x * progress,
						view.size.y - fullsize.y * 0.5f - offset + bottomOffset
					);

					renderTarget.draw(dot);
				}
			}
		}
	}

	if (hasError)
	{
		sf::Text errorText("", *font->getResource());
		errorText.setOutlineThickness(2.f);

		if (current.image != nullptr)
		{
			const String &errorStr = current.image->getErrorText();
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

		if (numImages == 0)
		{
			errorText.setString(TS_WFMT(
				"Path: %s",
				viewerManager->getFilepath()
			));

			bounds = errorText.getLocalBounds();
			errorText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
			errorText.setPosition(view.size.x / 2.f, view.size.y / 2.f + 30.f);
			errorText.setScale(0.55f, 0.55f);

			renderTarget.draw(errorText);
		}
	}

	if (showManagerStatus || showSchedulerStatus)
	{
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

void ImageViewerScene::drawLoaderGadget(sf::RenderTarget &renderTarget, const math::VC2 &centerPosition, float width)
{
	sf::CircleShape c(5.f);

	float t = elapsedTimer.getElapsedTime().getSecondsAsFloat();

	math::VC2 position = centerPosition;
	position.x += std::cos(t * 2.f) * width;
	position.y += -math::abs(std::cos(t * 10.f) * 10.f) * ((std::cos(t * 4.f - math::PI) + 1.f) * 0.5f);

	c.setPosition(position);
	c.setFillColor(sf::Color::White);
	c.setOutlineColor(sf::Color::Black);

	renderTarget.draw(c);
}

TS_END_PACKAGE2()
