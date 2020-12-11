#include "Precompiled.h"
#include "ts/ivie/scenes/ImageViewerScene.h"

#include "ts/lang/time/GlobalTimer.h"
#include "ts/file/FileUtils.h"
#include "ts/profiling/ZoneProfiler.h"
#include "ts/engine/window/WindowViewManager.h"
#include "ts/thread/ThreadScheduler.h"

#include "ts/ivie/util/NaturalSort.h"
#include "ts/ivie/util/RenderUtil.h"
#include "ts/ivie/viewer/ViewerManager.h"
#include "ts/ivie/viewer/ViewerEventManager.h"

#if TS_PLATFORM == TS_WINDOWS

	#include "ts/lang/common/WindowsUtils.h"

#elif TS_PLATFORM == TS_LINUX

	#include "ts/lang/common/LinuxUtils.h"

#endif

// TS_PACKAGE2(app, scenes)
namespace ts { namespace app { namespace scenes {

static const math::COL ColorSuccess  (0.3f, 1.0f, 0.1f);
static const math::COL ColorWarning  (1.0f, 0.9f, 0.1f);
static const math::COL ColorError    (1.0f, 0.2f, 0.1f);
static const math::COL ColorInfo     (0.4f, 0.9f, 1.0f);
static const math::COL ColorInfoLight(0.7f, 0.95f, 1.0f);

ImageViewerScene::ImageViewerScene(engine::system::BaseApplication *application, DisplayMode defaultDisplayMode)
	: AbstractSceneBase(application)
	, pendingDisplayMode(defaultDisplayMode)
{
}

ImageViewerScene::~ImageViewerScene()
{
}

bool ImageViewerScene::start()
{
	TS_ZONE();

	defaultScale.reset(1.f, 20.f);
	imageScale.reset(1.f, 20.f);
	positionOffset.reset(math::VC2::zero, 30.f);

	windowManager = &getGigaton<engine::window::WindowManager>();
	
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
	gaussianShader = rm.loadShader("gaussian_shader", "shader/gaussian.frag", true);
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
	const bool shiftDown = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

	switch (event.type)
	{
// 		case sf::Event::KeyPressed:
// 		{
// 			switch (event.key.code)
// 			{
// 				default: /* bop */ break;
// 			}
// 		}
// 		break;

		case sf::Event::MouseButtonPressed:
		{
			switch (event.mouseButton.button)
			{
				case sf::Mouse::Left:
				{
					if (displayMode == Normal || ctrlDown)
					{
						clickTimer.restart();
						dragged = 0.f;
					}
				}
				return true;

				case sf::Mouse::Right:
				{
					if (displayMode == Normal || ctrlDown)
					{
						imageScale.setTarget(1.f);
						positionOffset.setTarget(math::VC2::zero);
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
						TS_ASSERT(defaultScale.getValue() > 0.f);
						imageScale.setTarget(1.f / defaultScale.getValue());

						{
							math::VC2 mouse(
								(float)event.mouseButton.x,
								(float)event.mouseButton.y
							);

							const engine::window::WindowView &view = windowManager->getApplicationView();

							float currentScale = defaultScale.getValue() * imageScale.getValue();
							math::VC2 diff = calculateMouseDiff(view, mouse, currentScale, 1.f);

							float distanceFromEdgeX = math::abs(mouse.x - (view.size.x / 2.f)) / (view.size.x / 2.f);
							float distanceFromEdgeY = math::abs(mouse.y - (view.size.y / 2.f)) / (view.size.y / 2.f);
							float distanceFromEdge = math::max(distanceFromEdgeX, distanceFromEdgeY);
							float multiplier = (float)std::pow((distanceFromEdge - 0.5f) * 2.f, 6) * 0.4f + 1.f;

							positionOffset.setTarget(positionOffset.getValue() + diff * multiplier);

							enforceOversizeLimits(defaultScale.getValue() * math::max(imageScale.getTarget(), imageScale.getValue()));
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
				positionOffset.setTarget(positionOffset.getTarget() + mouseDelta);
				enforceOversizeLimits(defaultScale.getValue() * imageScale.getValue());
			}
			
			if (sf::Mouse::isButtonPressed(sf::Mouse::Middle))
			{
				// Scales image by holding down middle mouse: moving mouse pointer up or to the right enlarges, and vice versa
				int32 deltaSign = math::abs(mouseDelta.x) > math::abs(mouseDelta.y) ? math::sign(mouseDelta.x) : -math::sign(mouseDelta.y);
				float delta = mouseDelta.length() * deltaSign;
				
				float targetScale = math::clamp(imageScale.getTarget() + (delta / 140.f) * imageScale.getTarget(), 0.9f, 10.f);
				imageScale.setTarget(targetScale);
				
				enforceOversizeLimits(defaultScale.getValue() * imageScale.getValue());
			}

			lastMousePosition = mousePosition;
		}
		break;

		case sf::Event::MouseWheelScrolled:
		{
			float delta = event.mouseWheelScroll.delta;

			DisplayMode zoomMode = displayMode;
			if (ctrlDown)
				zoomMode = Normal;

			switch (zoomMode)
			{
				case Normal:
				{
					{
						float targetScale = math::clamp(imageScale.getTarget() + delta * 0.15f * imageScale.getTarget(), 1.f, 10.f);
						imageScale.setTarget(targetScale);
					}

					{
						math::VC2 mouse(
							(float)event.mouseWheelScroll.x,
							(float)event.mouseWheelScroll.y
						);

						float currentScale = defaultScale.getValue() * imageScale.getValue();
						float targetScale = defaultScale.getValue() * imageScale.getTarget();

						const engine::window::WindowView &view = windowManager->getApplicationView();
						math::VC2 diff = calculateMouseDiff(view, mouse, currentScale, targetScale);

						float multiplier = 1.f;
						if (delta > 0.f)
						{
							float distanceFromEdgeX = math::abs(mouse.x - (view.size.x / 2.f)) / (view.size.x / 2.f);
							float distanceFromEdgeY = math::abs(mouse.y - (view.size.y / 2.f)) / (view.size.y / 2.f);
							float distanceFromEdge = math::max(distanceFromEdgeX, distanceFromEdgeY);
							multiplier = (float)std::pow((distanceFromEdge - 0.5f) * 2.f, 4) * 0.13f + 1.f;
						}
						
						// Here it's important the diff is added to _current_ value, not the target,
						// otherwise the target position would grow out of hand.
						positionOffset.setTarget(positionOffset.getValue() + diff * multiplier * targetScale);
						
						enforceOversizeLimits(defaultScale.getValue() * math::max(imageScale.getTarget(), imageScale.getValue()));
					}
				}
				break;

				case Manga:
				{
					positionOffset.setTarget(positionOffset.getTarget() + math::VC2(0.f, delta * 140.f));
					enforceOversizeLimits(defaultScale.getValue() * imageScale.getTarget());
				}
				break;
			}
		}
		break;
		
		default: /* bop */ break;
	}

	return false;
}

void ImageViewerScene::handleInput(const input::InputManager &input)
{
	const bool ctrlDown = input.isKeyPressed(Keyboard::LControl) || input.isKeyPressed(Keyboard::RControl);
	const bool shiftDown = input.isKeyPressed(Keyboard::LShift) || input.isKeyPressed(Keyboard::RShift);

	if (changeTimer.getElapsedTime() >= 300_ms)
	{
		bool changed = false;

		if (input.isKeyPressed(Keyboard::Left))
		{
			viewerManager->changeImage(-1 * (ctrlDown ? 10 : 1));
			changed = true;
		}
		else if (input.isKeyPressed(Keyboard::Right))
		{
			viewerManager->changeImage(1 * (ctrlDown ? 10 : 1));
			changed = true;
		}
		else if (input.isKeyPressed(Keyboard::PageUp))
		{
			viewerManager->changeImage(-10);
			changed = true;
		}
		else if (input.isKeyPressed(Keyboard::PageDown))
		{
			viewerManager->changeImage(10);
			changed = true;
		}

		if (changed)
			changeTimer.restart();
	}
	else
	{
		if (!input.isKeyPressed(Keyboard::Left) &&
			!input.isKeyPressed(Keyboard::Right) &&
			!input.isKeyPressed(Keyboard::PageUp) &&
			!input.isKeyPressed(Keyboard::PageDown))
		{
			// In effect clears the timer
			changeTimer.set(1000_ms);
		}
	}
	
	if (input.wasKeyPressed(Keyboard::Add) || input.wasKeyPressed(Keyboard::Subtract))
	{
		int32 dir = (input.wasKeyPressed(Keyboard::Subtract) ? -1 : 1);

		float targetScale = math::clamp(imageScale.getTarget() + dir * 0.15f * imageScale.getTarget(), 0.9f, 10.f);
		imageScale.setTarget(targetScale);
	}

	if (input.wasKeyPressed(Keyboard::S))
	{
		uint16 style = (uint16)viewerManager->getSortingStyle();
		bool reversed = viewerManager->getSortingReversed();

		if (!shiftDown)
		{
			style = (style + 1) % viewer::SortingStyle_NumOptions;
			reversed = false;
		}
		else
		{
			reversed = !reversed;
		}

		viewerManager->setSorting((viewer::SortingStyle)style, reversed);

		String sortingStyle = "not set";
		String sortingOrder = !reversed ? "(ascending)" : "(descending)";

		switch ((viewer::SortingStyle)style)
		{
			case viewer::SortingStyle_ByName:
				sortingStyle = "name";
			break;
			case viewer::SortingStyle_ByType:
				sortingStyle = "type";
			break;
			case viewer::SortingStyle_ByLastModified:
				sortingStyle = "last modified";
			break;
			default: break;
		}
		
		addEventNotification(
			TS_FMT("Sorting by %s %s", sortingStyle, sortingOrder), ColorInfoLight);
	}

	if (input.wasKeyPressed(Keyboard::P))
	{
		displaySmooth = !displaySmooth;
	}

	if (input.wasKeyPressed(Keyboard::F1))
	{
		showManagerStatus = !showManagerStatus;
	}

	if (input.wasKeyPressed(Keyboard::F2))
	{
		showSchedulerStatus = !showSchedulerStatus;
	}

	if (input.wasKeyPressed(Keyboard::F5))
	{
		if (current.image != nullptr)
			current.image->reload();
	}

	if (input.wasKeyPressed(Keyboard::F6))
	{
		backgroundShader->reloadResource();
	}

	if (input.wasKeyPressed(Keyboard::G))
	{
#if TS_PLATFORM == TS_WINDOWS
		windows::openExplorerToFile(viewerManager->getCurrentFilepath());
#elif TS_PLATFORM == TS_LINUX
		linux::openExplorerToFile(viewerManager->getCurrentFilepath());
#endif
	}

	if (input.wasKeyPressed(Keyboard::D))
	{
#if TS_PLATFORM == TS_WINDOWS
		windows::openFileWithDialog(viewerManager->getCurrentFilepath());
#endif
	}

	if (input.wasKeyPressed(Keyboard::M))
	{
		setDisplayMode(displayMode == Normal ? Manga : Normal);
	}

	if (input.wasKeyPressed(Keyboard::R))
	{
		bool recursiveState = !viewerManager->getIsRecursiveScan();
		viewerManager->setRecursiveScan(recursiveState);

		if (recursiveState)
		{
			addEventNotification("Recursive scan enabled", ColorInfo);
		}
		else
		{
			addEventNotification("Recursive scan disabled", ColorInfo);
		}
	}

	if (input.wasKeyPressed(Keyboard::H))
	{
		viewerInfoMode = viewerInfoMode == ViewerInfo_DisplayAll ? ViewerInfo_HideAll : ViewerInfo_DisplayAll;
	}

	if (input.wasKeyPressed(Keyboard::Delete) && shiftDown)
	{
		bool success = viewerManager->deleteCurrentImage();
		if (success)
		{
			addEventNotification("Image deleted", ColorInfo);
		}
		else
		{
			addEventNotification("Delete failed", ColorError);
		}
	}

	if (ctrlDown && shiftDown && input.wasKeyPressed(Keyboard::C))
	{
		const String filename = viewerManager->getCurrentFilepath(true);
		sf::Clipboard::setString(filename);
		addEventNotification("Image path copied", ColorInfo);
	}
	
	if (current.image != nullptr && current.hasData && !current.hasError)
	{
		const bool rotateLeftPressed = input.wasKeyPressed(Keyboard::Comma);
		const bool rotateRightPressed = input.wasKeyPressed(Keyboard::Period);

		if (rotateLeftPressed || rotateRightPressed)
		{
			if (current.image->canImageBeRotated())
			{
				bool success = false;
				if (rotateLeftPressed)
				{
					success = viewerManager->rotateCurrentImage(image::Image::Rotation_CounterClockwise);
					if (success)
						addEventNotification("Rotated counter-clockwise", ColorSuccess);
				}
				else if (rotateRightPressed)
				{
					success = viewerManager->rotateCurrentImage(image::Image::Rotation_Clockwise);
					if (success)
						addEventNotification("Rotated clockwise", ColorSuccess);
				}

				if (!success)
					addEventNotification("Rotation failed", ColorError);
			}
			else
			{
				addEventNotification("Not rotatable", ColorWarning);
			}
		}
	}

	if (input.wasKeyPressed(Keyboard::Z))
	{
		rotationInfo.rotation = DisplayRotation(((int32)rotationInfo.rotation + 3) % 4);
		rotationInfo.visualRotation.setTarget(rotationInfo.visualRotation.getTarget() - 90.f);

		imageScale.setTarget(1.f);
		positionOffset.setTarget(math::VC2::zero);

		updateDefaultScale();
		enforceOversizeLimits(defaultScale.getValue() * imageScale.getTarget());
	}
	else if(input.wasKeyPressed(Keyboard::X))
	{
		rotationInfo.rotation = DisplayRotation(((int32)rotationInfo.rotation + 1) % 4);
		rotationInfo.visualRotation.setTarget(rotationInfo.visualRotation.getTarget() + 90.f);

		imageScale.setTarget(1.f);
		positionOffset.setTarget(math::VC2::zero);

		updateDefaultScale();
		enforceOversizeLimits(defaultScale.getValue() * imageScale.getTarget());
	}

	if (input.wasKeyPressed(Keyboard::Q))
	{
		application->requestQuit();
	}
}

math::VC2 ImageViewerScene::transformSizeByDisplayRotation(const math::VC2 &vec) const
{
	switch (rotationInfo.rotation)
	{
	case DisplayRotation::Top:
	case DisplayRotation::Bottom:
		return math::VC2(vec.x, vec.y);

	case DisplayRotation::Right:
	case DisplayRotation::Left:
		return math::VC2(vec.y, vec.x);
	}
	return vec;
}

void ImageViewerScene::updateDefaultScale()
{
	if (current.hasData)
	{
		math::VC2 imageSize = static_cast<math::VC2>(current.data.size);
		imageSize = transformSizeByDisplayRotation(imageSize);

		const math::VC2 viewportSize = viewport.getSize();
		float targetScale = math::min(
			math::min(1.f, (viewportSize.x - framePadding) / imageSize.x),
			math::min(1.f, (viewportSize.y - framePadding) / imageSize.y)
		);
		defaultScale.setTarget(targetScale);
	}
}

bool ImageViewerScene::updateImageInfo()
{
	TS_ZONE();

	TS_ASSERT(current.image != nullptr);

	image::ImageData imageData;
	if (current.image->getImageData(imageData))
	{
		current.data = std::move(imageData);
		TS_ASSERT(current.data.size.x > 0 && current.data.size.y > 0);

		current.hasData = true;

// 		const engine::window::WindowView &view = windowManager->getApplicationView();

		updateDefaultScale();
		defaultScale.cutToTarget();

		if (displayMode != pendingDisplayMode)
		{
			setDisplayMode(pendingDisplayMode);
		}

		switch (displayMode)
		{
		case Normal:
		{
			imageScale.setTarget(1.f);
			if (defaultScale.getValue() == 1.f)
				imageScale.cutToTarget();

			positionOffset.setTarget(math::VC2::zero);
		}
		break;

		case Manga:
		{
			positionOffset.setTarget(math::VC2(0.f, 10000.f));
			enforceOversizeLimits(defaultScale.getValue() * imageScale.getTarget());
		}
		break;
		}

		String filepath = current.image->getFilepath();
		String filename = file::getBasename(filepath);

		windowManager->setWindowTitle(TS_WFMT("%s  |  %s", filename, getApplicationDefaultWindowTitle()));

		current.currentDirname = file::getBasename(file::getDirname(filepath));
		current.currentDirname.truncate(40, "...");
		if (!viewerManager->getImageIndexForCurrentDirectory(current.indexInDirectory, current.numImagesInDirectory))
		{
			current.indexInDirectory = 0;
			current.numImagesInDirectory = 0;
		}

		frameTimer.restart();

		return true;
	}
	else if (current.image->hasError())
	{
		current.hasError = true;
		
		defaultScale.setTarget(1.f);
		defaultScale.cutToTarget();
		
		imageScale.setTarget(1.f);
		imageScale.cutToTarget();
		
		positionOffset.setTarget(math::VC2::zero);
		positionOffset.cutToTarget();

		windowManager->setWindowTitle(getApplicationDefaultWindowTitle());
		
		return true;
	}
	return false;
}

void ImageViewerScene::update(const TimeSpan deltaTime)
{
	TS_ZONE();

	const engine::window::WindowView &view = windowManager->getApplicationView();

	framePadding = math::max(20.f, view.size.x * 0.02f);
	updateViewport(view);

	if (current.image != nullptr && !current.hasError && !current.hasData)
		updateImageInfo();

	if (!sf::Mouse::isButtonPressed(sf::Mouse::Middle))
	{
		imageScale.setTarget(math::max(1.f, imageScale.getTarget()));
	}

	if (windowManager->isInFocus())
	{
		switch (displayMode)
		{
			case Manga:
			{
				const bool upPressed   = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
				const bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);

				if (upPressed || downPressed)
				{
					const float delta = (upPressed ? 1.f : -1.f);
					const math::VC2 move = math::VC2(0.f, delta * 1400.f * deltaTime.getSecondsAsFloat());
					positionOffset.setTarget(positionOffset.getTarget() + move);
					enforceOversizeLimits(defaultScale.getValue() * imageScale.getTarget());
				}
			}
			break;

			default: break;
		}
	}

	if (!deleteWasReleased && !sf::Keyboard::isKeyPressed(sf::Keyboard::Delete))
		deleteWasReleased = true;

	updateEventNotifications(deltaTime);
}

void ImageViewerScene::updateFrequent(const TimeSpan deltaTime)
{
	TS_ZONE();

	rotationInfo.visualRotation.update(deltaTime);

	defaultScale.update(deltaTime);

	imageScale.update(deltaTime);

	float positionSmoothing = math::abs(imageScale.getTarget() - imageScale.getValue()) < 0.01f
		? 30.f : imageScale.getSmoothingFactor();
	positionOffset.setSmoothingFactor(positionSmoothing);
	positionOffset.update(deltaTime);

	float deltaSeconds = deltaTime.getSecondsAsFloat();
	{
		float targetIndexAlpha = viewerInfoMode != ViewerInfo_HideAll ? 1.f : 0.f;
		viewerInfoAlpha.index += (targetIndexAlpha - viewerInfoAlpha.index) * deltaSeconds * 16.f;
		viewerInfoAlpha.index = math::clamp(viewerInfoAlpha.index, 0.f, 1.f);

		float targetOtherAlpha = viewerInfoMode == ViewerInfo_DisplayAll ? 1.f : 0.f;
		viewerInfoAlpha.other += (targetOtherAlpha - viewerInfoAlpha.other) * deltaSeconds * 16.f;
		viewerInfoAlpha.other = math::clamp(viewerInfoAlpha.other, 0.f, 1.f);
	}

	enforceOversizeLimits(defaultScale.getValue() * imageScale.getValue(), false);
}

void ImageViewerScene::updateViewport(const engine::window::WindowView &view)
{
	viewport.minbounds = math::VC2(0.f, 50.f);
	viewport.maxbounds = math::VC2(view.size.x, view.size.y);
}

void ImageViewerScene::enforceOversizeLimits(float scale, bool enforceTarget)
{
	if (!current.hasData)
		return;

	math::VC2 scaledSize = static_cast<math::VC2>(current.data.size) * scale;
	scaledSize = transformSizeByDisplayRotation(scaledSize);

	math::VC2 viewportSize = viewport.getSize();

	math::VC2 oversize = (scaledSize - viewportSize) * 0.5f;
	oversize.x = math::max(0.f, oversize.x);
	oversize.y = math::max(0.f, oversize.y);

	positionOversizeLimit = oversize;

	if (enforceTarget)
	{
		math::VC2 targetOffset = positionOffset.getTarget();
		targetOffset.x = math::clamp(targetOffset.x, -oversize.x, oversize.x);
		targetOffset.y = math::clamp(targetOffset.y, -oversize.y, oversize.y);
		positionOffset.setTarget(targetOffset);
	}
	
	math::VC2 offset = positionOffset.getValue();
	offset.x = math::clamp(offset.x, -oversize.x, oversize.x);
	offset.y = math::clamp(offset.y, -oversize.y, oversize.y);
	positionOffset.setValue(offset);
}

math::VC2 ImageViewerScene::calculateMouseDiff(const engine::window::WindowView &view,
	const math::VC2 &mousePos, float currentScale, float targetScale)
{
	if (!current.hasData)
		return math::VC2::zero;

	math::VC2 imageSize = static_cast<math::VC2>(current.data.size);
	imageSize = transformSizeByDisplayRotation(imageSize);

	math::VC2 converted = view.convertToViewCoordinate(mousePos);
	math::VC2 imageCenter = static_cast<math::VC2>(imageSize) * 0.5f;

	math::VC2 viewportOffset = viewport.getCenter() - (view.size * 0.5f);

	math::Transform currentTransform;
	currentTransform
		.rotate(rotationInfo.visualRotation.getValue())
		.translate(positionOffset.getValue() + viewportOffset)
		.scale(currentScale, currentScale)
		.translate(-imageCenter);

	math::Transform targetTransform;
	targetTransform
		.rotate(rotationInfo.visualRotation.getValue())
		.translate(positionOffset.getValue() + viewportOffset)
		.scale(targetScale, targetScale)
		.translate(-imageCenter);

	math::VC2 currentMouseImagePos = currentTransform.getInverse().transformPoint(converted);
	math::VC2 targetMouseImagePos = targetTransform.getInverse().transformPoint(converted);

	return targetMouseImagePos - currentMouseImagePos;
}

void ImageViewerScene::imageChanged(SharedPointer<image::Image> image)
{
	TS_ZONE();

	rotationInfo.rotation = DisplayRotation::Top;
	rotationInfo.visualRotation.reset(0.f, 30.f);

	current = CurrentState();
	current.image = image;

	if (current.image != nullptr && !current.hasError && !current.hasData)
		updateImageInfo();
}

void ImageViewerScene::filelistChanged(SizeType numFiles)
{
	if (numFiles == 0)
		current.image = nullptr;
}

void ImageViewerScene::screenResized(const math::VC2U &size)
{
	const engine::window::WindowView &view = windowManager->getApplicationView();
	updateViewport(view);
	updateDefaultScale();
}

void ImageViewerScene::filesDropped(const std::vector<engine::window::DroppedFile> &files)
{
	if (files.empty())
		return;

	viewerManager->setViewerPath(files[0].filepath);
}

void ImageViewerScene::renderApplication(sf::RenderTarget &renderTarget, const engine::window::WindowView &view)
{
	TS_ZONE();

	renderTarget.clear(math::COL(0.12f, 0.12f, 0.12f));

	if (backgroundShader && backgroundShader->isLoaded())
	{
		sf::RectangleShape bg(view.size);
		bg.setFillColor(math::COL(0.12f, 0.12f, 0.12f));
		bg.setOrigin(view.size / 2.f);

		sf::Shader &bgShader = *backgroundShader->getResource();
		bgShader.setUniform("u_resolution", view.size);

		renderTarget.draw(bg, &bgShader);
	}

	if (current.image != nullptr)
	{
		const float scale = defaultScale.getValue() * imageScale.getValue();
		const math::VC2 scaledSize = static_cast<math::VC2>(current.data.size) * scale;
		const math::VC2 viewportOffset = viewport.getCenter() - (view.size * 0.5f);

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

				const math::VC2 offset = positionOffset.getValue() + viewportOffset;

				math::Transform transform;
				transform
					.translate(offset)
					.rotate(rotationInfo.visualRotation.getValue())
					.translate(scaledSize * -0.5f)
					.scale(scale, scale);

				states.transform = (sf::Transform)transform;

				image::DisplayShaderParams params;
				params.viewSize = view.size;
				params.scale = scale;
				params.offset = offset;
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
				const math::VC2U thumbnailSize = thumbnail->getSize();

				sf::VertexArray va = util::makeQuadVertexArrayScaled(
					current.data.size.x, current.data.size.y,
					thumbnailSize.x, thumbnailSize.y
				);

				sf::RenderStates states;
				states.texture = thumbnail.get();

				const math::VC2 offset = positionOffset.getValue() + viewportOffset;
				math::Transform transform;
				transform
					.translate(offset)
					.rotate(rotationInfo.visualRotation.getValue())
					.translate(scaledSize * -0.5f)
					.scale(scale, scale);

				states.transform = (sf::Transform)transform;

				sf::Shader &gaussian = *gaussianShader->getResource();
// 				gaussian.setUniform("u_textureSize", static_cast<math::VC2>(thumbnailSize));
				gaussian.setUniform("u_textureSize", static_cast<math::VC2>(current.data.size));
				gaussian.setUniform("u_direction", math::VC2(0.5f, 0.f));
				states.shader = &gaussian;

				renderTarget.draw(va, states);
			}
		}
	}
}

void ImageViewerScene::renderInterface(sf::RenderTarget &renderTarget, const engine::window::WindowView &view)
{
	TS_ZONE();

	const SizeType numImages = viewerManager->getNumImages();
	const bool hasImages = numImages > 0;

	const bool hasError = [&]()
	{
		if (current.image != nullptr && numImages > 0)
			return current.image->hasError();

		return true;
	}();

	if (!hasError)
	{
		if (current.image != nullptr && numImages > 0 && !current.image->isDisplayable())
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
		sf::Text errorText("", *font->getResource());
		errorText.setOutlineThickness(2.f);

		if (hasImages && current.image != nullptr)
		{
			const String &errorStr = current.image->getErrorText();
			errorText.setString(TS_WFMT(
				"Error: %s", (!errorStr.isEmpty() ? errorStr : "Unknown error (not set).")
			));
		}
		else if (!hasImages)
		{
			if (viewerManager->isFirstScanComplete())
				errorText.setString("Directory has no displayable files.");
			else
				errorText.setString("Scanning files...");
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
				viewerManager->getViewerPath()
			));

			bounds = errorText.getLocalBounds();
			errorText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
			errorText.setPosition(view.size.x / 2.f, view.size.y / 2.f + 30.f);
			errorText.setScale(0.55f, 0.55f);

			renderTarget.draw(errorText);
		}
	}

	if (viewerInfoAlpha.index >= 0.003f && numImages > 0)
	{
		sf::Text statusText("", *font->getResource(), 26);
		statusText.setOutlineThickness(2.f);

		float bottomOffset = (1.f - viewerInfoAlpha.other) * 60.f;
		float textBottomOffset = 27.f - bottomOffset;

		statusText.setFillColor(math::COL(1.f, 1.f, 1.f, viewerInfoAlpha.index));
		statusText.setOutlineColor(math::COL(0.f, 0.f, 0.f, viewerInfoAlpha.index));

		// Draw bottom status info
		if (viewerInfoAlpha.other >= 0.03f)
		{
			statusText.setFillColor(math::COL(1.f, 1.f, 1.f, viewerInfoAlpha.index));
			statusText.setOutlineColor(math::COL(0.f, 0.f, 0.f, viewerInfoAlpha.index));

			const String filename = viewerManager->getCurrentFilepath(true);
			if (!filename.isEmpty())
			{
				{
					String basename = file::getBasename(filename);

					statusText.setString(basename);
					statusText.setOrigin(statusText.getLocalBounds().width, 0.f);

					float filenameScale = math::clamp(50.f / (float)basename.getSize(), 0.75f, 1.f) * 0.8f;
					statusText.setScale(filenameScale, filenameScale);

					statusText.setPosition(
						view.size.x - 26.f,
						view.size.y - 30.f * filenameScale - textBottomOffset
					);

					renderTarget.draw(statusText);
				}
			}

			statusText.setFillColor(math::COL(1.f, 1.f, 1.f, viewerInfoAlpha.other));
			statusText.setOutlineColor(math::COL(0.f, 0.f, 0.f, viewerInfoAlpha.other));

			if (!hasError)
			{
				statusText.setOrigin(0.f, 0.f);

				float scale = defaultScale.getValue() * imageScale.getValue();

				statusText.setString(TS_FMT("%.1f%%\n%u x %u",
					scale * 100.f,
					current.data.size.x, current.data.size.y
				));

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
					bar.setFillColor(math::COL(0, 0, 0, viewerInfoAlpha.other * 0.63f));
					bar.setOutlineColor(math::COL(0.3f, 0.3f, 0.3f, viewerInfoAlpha.other * 0.78f));
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
					bar.setFillColor(math::COL(1.f, 1.f, 1.f, viewerInfoAlpha.other * 0.9f));

					bar.setOrigin(0.f, fullsize.y);
					bar.setPosition(
						view.size.x / 2.f - fullsize.x / 2.f,
						view.size.y - offset + bottomOffset
					);

					renderTarget.draw(bar, sf::BlendAdd);
				}

				{
					sf::CircleShape dot(5.f);
					dot.setFillColor(math::COL(1.f, 0.86f, 0.16f, viewerInfoAlpha.other));
					dot.setOutlineColor(math::COL(0.5f, 0.43f, 0.8f, viewerInfoAlpha.other * 0.5f));
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

	// TOP BAR
	{
		{
			sf::RectangleShape topBar(math::VC2(view.size.x, 49.f));
			topBar.setFillColor(math::COL(0.f, 0.f, 0.f, 0.5f));
			topBar.setOutlineColor(math::COL(1.f, 1.f, 1.f, 0.4f));
			topBar.setOutlineThickness(1.f);
			topBar.setPosition(math::VC2::zero);
			renderTarget.draw(topBar);
		}

		sf::Text statusText("", *font->getResource(), 26);
		statusText.setOutlineThickness(2.f);

		statusText.setFillColor(math::COL(1.f, 1.f, 1.f, 1.f));
		statusText.setOutlineColor(math::COL(0.f, 0.f, 0.f, 1.f));
		
		// Draw image index
		if (hasImages)
		{
			statusText.setString(TS_WFMT("%u / %u",
				viewerManager->getCurrentImageIndex() + 1,
				viewerManager->getNumImages()
			));

			statusText.setOrigin(0.f, statusText.getLocalBounds().height * 0.5f);

			statusText.setPosition(
				16.f,
				23.f
			);
			statusText.setScale(0.8f, 0.8f);

			renderTarget.draw(statusText);

			if (viewerManager->isScanningFiles() && !viewerManager->isFirstScanComplete())
			{
				math::FloatRect bounds = statusText.getGlobalBounds();

				math::VC2 position(bounds.maxbounds.x + 30.f, bounds.getCenter().y - 1.f);
				drawLoaderGadget(renderTarget, position, 12.f, 4.f);
			}
		}

		// Draw current dir name
		if (hasImages)
		{
			const String filename = viewerManager->getCurrentFilepath(true);
			if (!filename.isEmpty())
			{
				if (current.numImagesInDirectory > 0)
				{
					statusText.setString(TS_WFMT(
						"%s (%u / %u)",
						current.currentDirname, current.indexInDirectory + 1, current.numImagesInDirectory
					));
				}
				else
				{
					statusText.setString(current.currentDirname);
				}
				statusText.setOrigin(statusText.getLocalBounds().width, statusText.getLocalBounds().height * 0.5f);

// 				float filenameScale = math::clamp(45.f / (float)dirname.getSize(), 0.5f, 1.f) * 0.7f;
				float filenameScale = 0.7f;
				statusText.setScale(filenameScale, filenameScale);

				statusText.setPosition(
					view.size.x - 26.f,
					23.f
				);

				math::COL statusColor(0xfffcc5ff);
// 				statusColor.a = viewerInfoAlpha.index;
				statusText.setFillColor(statusColor);

				renderTarget.draw(statusText);
			}
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

	drawEventNotifications(renderTarget, view, math::VC2(-30.f, 65.f));
}

void ImageViewerScene::setDisplayMode(const DisplayMode modeParam)
{
	if (displayMode == modeParam)
		return;

	pendingDisplayMode = displayMode = modeParam;

	switch (modeParam)
	{
	case Normal:
	{
		imageScale.setTarget(1.f);
		addEventNotification("Normal mode", ColorInfo);
	}
	break;

	case Manga:
	{
		float scaleToFit = 1.f;
		if (current.hasData)
			scaleToFit = (viewport.getSize().x / (float)current.data.size.x) * 0.9f;

		float scale = math::min(1.f, scaleToFit) / defaultScale.getValue();

		imageScale.setTarget(scale);
		positionOffset.setTarget(math::VC2(0.f, current.data.size.y * 2.f));
		enforceOversizeLimits(defaultScale.getValue() * imageScale.getTarget());

		viewerInfoMode = ViewerInfo_HideAll;
		viewerManager->setSorting(viewer::SortingStyle_ByName, false);

		addEventNotification("Manga mode", ColorInfo);
	}
	break;

	default: TS_ASSERT(!"Displaymode not supported"); break;
	}
}

void ImageViewerScene::drawLoaderGadget(sf::RenderTarget &renderTarget, const math::VC2 &centerPosition, float width, float size)
{
	const float t = elapsedTimer.getElapsedTime().getSecondsAsFloat();

	const float period = 1.5f;

	math::VC2 position = centerPosition;
	position.x += std::cos(t * math::PI / period) * width;
	position.y += -math::abs(std::cos(t * 10.f) * 10.f) * ((std::cos(t * 2.f * math::PI / period - math::PI) + 1.f) * 0.5f);

	const SizeType mode = (int32)std::fmod(t / period, 4.f);

	struct { float r, g, b; } mc;
	switch (mode)
	{
		case 0:  mc = { 0.f,  1.f,  1.f }; break; // red
		case 1:  mc = { 0.f,  0.3f, 1.f }; break; // yellow
		case 2:  mc = { 0.8f, 0.2f, 1.f }; break; // green
		case 3:  mc = { 1.f,  0.4f, 0.f }; break; // blue
		default: mc = { 1.f,  1.f,  1.f }; break;
	}

#define sawtooth(t, a) ( 2.f * ( ((t) / (a)) - math::floor(0.5f + ((t) / (a))) ) )
	float modifier = sawtooth(t + period * 0.5f, -period) * 0.5f + 0.5f;
	float r = 1.f - modifier * mc.r;
	float g = 1.f - modifier * mc.g;
	float b = 1.f - modifier * mc.b;
#undef sawtooth

	sf::CircleShape circle(size);
	circle.setPosition(position);

	{
		math::COL c = math::COL(r, g, b, (modifier - 0.1f) * 0.8f);
		circle.setOutlineColor(c);
		circle.setOutlineThickness(10.f - modifier * 8.f);
		renderTarget.draw(circle);
	}

	{
		math::COL c = math::COL(r, g, b);
		circle.setOutlineColor(c);
		circle.setFillColor(c * 0.45f);
		circle.setOutlineThickness(2.f);
		renderTarget.draw(circle);
	}
}

void ImageViewerScene::updateEventNotifications(TimeSpan delta)
{
	viewer::ViewerEventManager &em = getGigaton<viewer::ViewerEventManager>();

	viewer::ViewerEvent event;
	while (em.pollEvent(event))
	{
		switch (event.type)
		{
			case viewer::ViewerEvent::ImageDeleted:
				if (event.dataBool)
					addEventNotification("Deleted", math::COL(1.f, 0.3f, 0.3f));
				else
					addEventNotification("Delete failed", math::COL(1.f, 0.3f, 0.3f));
			break;

			case viewer::ViewerEvent::ModeChanged:
				if (event.dataBool)
					addEventNotification("Manga mode");
				else
					addEventNotification("Normal mode");
			break;
		}
	}

	Time currentTime = Time::now();
	for (std::vector<EventNotification>::iterator it = eventNotifications.begin(); it != eventNotifications.end();)
	{
		EventNotification &en = *it;
		if (currentTime >= en.expiry)
		{
			it = eventNotifications.erase(it);
		}
		else
		{
			en.offset = math::max(0.f, en.offset + ((0.f - en.offset) * delta.getSecondsAsFloat() * 35.f));
			++it;
		}
	}
}

void ImageViewerScene::drawEventNotifications(sf::RenderTarget &target, const engine::window::WindowView &view, const math::VC2 &drawOffset)
{
	sf::Text notificationText;
	notificationText.setOutlineThickness(2.f);
	notificationText.setFont(*font->getResource());
	notificationText.setCharacterSize(24);

	math::VC2 drawPosition(view.size.x + drawOffset.x, drawOffset.y);
	float rowOffset = 32.f;

	for (std::vector<EventNotification>::iterator it = eventNotifications.begin(); it != eventNotifications.end(); ++it)
	{
		const EventNotification &en = *it;

		TimeSpan timeUntilExpiry = en.expiry - Time::now();
		float eventProgress = 1.f - (float)(timeUntilExpiry / eventNotificationDuration);
		float progressMultiplier = std::sin(math::max(0.8333f, eventProgress) * math::PI * 3.f);
		progressMultiplier = math::clamp(progressMultiplier, 0.f, 1.f);

		notificationText.setString(en.text);
		
		math::COL color = en.color;
		color.a = progressMultiplier;
		notificationText.setFillColor(color);

		math::COL outline = math::COL::black;
		outline.a = progressMultiplier;
		notificationText.setOutlineColor(outline);

		sf::FloatRect rekt = notificationText.getLocalBounds();
		notificationText.setOrigin(rekt.width, 0.f);

		float horizontalOffset = en.offset / 100.f * (rekt.width + 30.f);

		math::VC2 position = drawPosition;
		position.x += horizontalOffset;

		notificationText.setPosition(position);
		notificationText.setScale(0.75f, 0.75f);

		target.draw(notificationText);

		drawPosition.y += rowOffset * progressMultiplier;
	}
}

void ImageViewerScene::addEventNotification(String text, math::COL color)
{
	EventNotification en;
	en.text = text;
	en.color = color;

	en.expiry = Time::now() + eventNotificationDuration;
	en.offset = 100.f;

	eventNotifications.push_back(std::move(en));

	if (eventNotifications.size() > 10)
	{
		SizeType excess = (SizeType)eventNotifications.size() - 10;
		std::vector<EventNotification>::iterator it = eventNotifications.begin();
		for (SizeType i = 0; i < excess; ++i)
		{
			it->expiry = Time::now();
			++it;
		}
	}
}

TS_END_PACKAGE2()
