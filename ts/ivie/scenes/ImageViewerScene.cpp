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

static const math::COL ColorSuccess(0.3f, 1.0f, 0.1f);
static const math::COL ColorWarning(1.0f, 0.9f, 0.1f);
static const math::COL ColorError  (1.0f, 0.2f, 0.1f);
static const math::COL ColorInfo   (0.1f, 0.9f, 1.0f);

ImageViewerScene::ImageViewerScene(engine::system::BaseApplication *application)
	: AbstractSceneBase(application)
{
}

ImageViewerScene::~ImageViewerScene()
{
}

bool ImageViewerScene::start()
{
	TS_ZONE();

	defaultScale.reset(1.f, 10.f);
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
// 				
// 
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
		SizeType option = (SizeType)viewerManager->getSorting();
		option = (option + 1) % viewer::SortingStyle_NumOptions;
		viewerManager->setSorting((viewer::SortingStyle)option);

		switch ((viewer::SortingStyle)option)
		{
			case viewer::SortingStyle_ByName:
				addEventNotification("Sorting by name", ColorInfo);
			break;
			case viewer::SortingStyle_ByExtension:
				addEventNotification("Sorting by extension", ColorInfo);
			break;
			default: break;
		}
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
		displayMode = displayMode == Normal ? Manga : Normal;
		switch (displayMode)
		{
			case Normal:
			{
				imageScale.setTarget(1.f);

				addEventNotification("Normal mode", ColorInfo);
			}
			break;

			case Manga:
			{
				imageScale.setTarget(1.f / defaultScale.getValue());
				positionOffset.setTarget(math::VC2(0.f, 10000.f));
				enforceOversizeLimits(defaultScale.getValue() * imageScale.getTarget());

				addEventNotification("Manga mode", ColorInfo);
			}
			break;
		}
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
		if (!shiftDown)
		{
			viewerInfoMode = viewerInfoMode == ViewerInfo_DisplayAll ? ViewerInfo_HideAll : ViewerInfo_DisplayAll;
		}
		else
		{
			viewerInfoMode = viewerInfoMode != ViewerInfo_IndexOnly ? ViewerInfo_IndexOnly : ViewerInfo_DisplayAll;
		}
	}

	if (input.wasKeyPressed(Keyboard::Delete) && shiftDown)
	{
		bool success = viewerManager->deleteCurrentImage();
		if (success)
		{
			addEventNotification("Image deleted", ColorSuccess);
		}
		else
		{
			addEventNotification("Delete failed", ColorError);
		}
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

	if (input.wasKeyPressed(Keyboard::Num1))
	{
		addEventNotification("Rotate LEFT");
		temporaryRotation.setTarget(temporaryRotation.getTarget() -  90.f);
		temporaryRotationQuat.makeFromAngleAxis(temporaryRotation.getTarget(), math::VC3(0.f, 0.f, 1.f));

		imageScale.setTarget(1.f);
		positionOffset.setTarget(math::VC2::zero);

		enforceOversizeLimits(defaultScale.getValue() * imageScale.getTarget());
	}
	else if(input.wasKeyPressed(Keyboard::Num2))
	{
		addEventNotification("Rotate RIGHT");
		imageScale.setTarget(1.f);
		positionOffset.setTarget(math::VC2::zero);

		temporaryRotation.setTarget(temporaryRotation.getTarget() + 90.f);
		temporaryRotationQuat.makeFromAngleAxis(temporaryRotation.getTarget(), math::VC3(0.f, 0.f, 1.f));

		enforceOversizeLimits(defaultScale.getValue() * imageScale.getTarget());
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
		current.hasData = true;

		const engine::window::WindowView &view = windowManager->getApplicationView();

		TS_ASSERT(current.data.size.x > 0 && current.data.size.y > 0);

		// float previousDefaultScale = defaultScale.getValue();

		float targetScale = math::min(
			math::min(1.f, (view.size.x - framePadding) / (float)current.data.size.x),
			math::min(1.f, (view.size.y - framePadding) / (float)current.data.size.y)
		);
		defaultScale.setTarget(targetScale);
		defaultScale.cutToTarget();

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
		
		return true;
	}
	return false;
}

void ImageViewerScene::update(const TimeSpan deltaTime)
{
	TS_ZONE();

	const engine::window::WindowView &view = windowManager->getApplicationView();

	framePadding = math::max(20.f, view.size.x * 0.02f);

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

	// const engine::window::WindowView &view = windowManager->getApplicationView();

	temporaryRotation.update(deltaTime);

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

void ImageViewerScene::enforceOversizeLimits(float scale, bool enforceTarget)
{
	if (!current.hasData)
		return;

	const engine::window::WindowView &view = windowManager->getApplicationView();

	math::VC2 scaledSize = static_cast<math::VC2>(current.data.size) * scale;
	scaledSize = temporaryRotationQuat.getRotated(scaledSize);

	math::VC2 oversize = (scaledSize - view.size) / 2.f;
	oversize.x = math::max(0.f, oversize.x);
	oversize.y = math::max(0.f, oversize.y);
	oversize = temporaryRotationQuat.getRotated(oversize);

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

	math::VC2 converted = view.convertToViewCoordinate(mousePos);
	math::VC2 imageCenter = static_cast<math::VC2>(current.data.size) / 2.f;

	math::Transform currentTransform;
	currentTransform
		.translate(positionOffset.getValue())
		.scale(currentScale, currentScale)
		.translate(-imageCenter);

	math::Transform targetTransform;
	targetTransform
		.translate(positionOffset.getValue())
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

	temporaryRotation.reset(0.f, 50.f);
	temporaryRotationQuat = math::Quat::identity;
}

void ImageViewerScene::filelistChanged(SizeType numFiles)
{
	if (numFiles == 0)
		current.image = nullptr;
}

void ImageViewerScene::screenResized(const math::VC2U &size)
{
	const engine::window::WindowView &view = windowManager->getApplicationView();

	float targetScale = math::min(
		math::min(1.f, (view.size.x - framePadding) / (float)current.data.size.x),
		math::min(1.f, (view.size.y - framePadding) / (float)current.data.size.y)
	);
	defaultScale.setTarget(targetScale);
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
		float scale = defaultScale.getValue() * imageScale.getValue();

		math::VC2 scaledSize = static_cast<math::VC2>(current.data.size) * scale;

		bool displayable = current.image->isDisplayable() && current.hasData && !sf::Keyboard::isKeyPressed(sf::Keyboard::Tab);
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

				math::Transform rotationTransform;
				rotationTransform.rotate(temporaryRotation.getValue());

				math::Transform transform;
				
				transform.combine(rotationTransform);

				math::VC2 poffset = rotationTransform.transformPoint(positionOffset.getValue());

				transform
					.translate(scaledSize * -0.5f + poffset)
					.scale(scale, scale);


				states.transform = (sf::Transform)transform;

				image::DisplayShaderParams params;
				params.viewSize = view.size;
				params.scale = scale;
				params.offset = positionOffset.getValue();
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

				sf::VertexArray va = util::makeQuadVertexArrayScaled(
					current.data.size.x, current.data.size.y,
					thumbnailSize.x, thumbnailSize.y
				);

				sf::RenderStates states;
				states.texture = thumbnail.get();

				math::Transform transform;
				transform
					.translate(scaledSize / -2.f + positionOffset.getValue())
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
	bool hasError = false;

	if (current.image != nullptr && numImages > 0)
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

		statusText.setFillColor(math::COL(1.f, 1.f, 1.f, viewerInfoAlpha.index));
		statusText.setOutlineColor(math::COL(0.f, 0.f, 0.f, viewerInfoAlpha.index));

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
			statusText.setFillColor(math::COL(1.f, 1.f, 1.f, viewerInfoAlpha.index));
			statusText.setOutlineColor(math::COL(0.f, 0.f, 0.f, viewerInfoAlpha.index));

			const String filename = viewerManager->getCurrentFilepath(false);
			if (!filename.isEmpty())
			{
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

				float scale = defaultScale.getValue() * imageScale.getValue();

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

	if (hasError)
	{
		sf::Text errorText("", *font->getResource());
		errorText.setOutlineThickness(2.f);

		if (current.image != nullptr && numImages > 0)
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
				viewerManager->getViewerPath()
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

	drawEventNotifications(renderTarget, view);
}

void ImageViewerScene::drawLoaderGadget(sf::RenderTarget &renderTarget, const math::VC2 &centerPosition, float width)
{
	sf::CircleShape c(5.f);

	float t = elapsedTimer.getElapsedTime().getSecondsAsFloat();

	math::VC2 position = centerPosition;
	position.x += std::cos(t * 2.f) * width;
	position.y += -math::abs(std::cos(t * 10.f) * 10.f) * ((std::cos(t * 4.f - math::PI) + 1.f) * 0.5f);

	c.setPosition(position);
	c.setFillColor(math::COL::white);
	c.setOutlineColor(math::COL::black);

	renderTarget.draw(c);
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

void ImageViewerScene::drawEventNotifications(sf::RenderTarget &target, const engine::window::WindowView &view)
{
	sf::Text notificationText;
	notificationText.setOutlineThickness(2.f);
	notificationText.setFont(*font->getResource());

	math::VC2 drawPosition(view.size.x - 30.f, 25.f);
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
