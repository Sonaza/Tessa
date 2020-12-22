#pragma once

#include "ts/engine/system/AbstractSceneBase.h"

#include "ts/math/Damper.h"
#include "ts/ivie/image/Image.h"

#include "ts/engine/window/WindowManager.h"

TS_DECLARE2(app, viewer, ViewerManager);
TS_DECLARE_STRUCT2(engine, system, WindowView);

// TS_PACKAGE2(app, scenes)
namespace ts { namespace app { namespace scenes {

class ImageViewerScene : public engine::system::AbstractSceneBase
{
	TS_DECLARE_SCENE(ImageViewerScene);

public:
	enum DisplayMode
	{
		Normal,
		Manga,
		NoReset,
	};

	ImageViewerScene(engine::system::BaseApplication *application, DisplayMode defaultDisplayMode);
	virtual ~ImageViewerScene();

	virtual bool start() override;
	virtual void stop() override;

	virtual void loadResources(resource::ResourceManager &rm) override;

	virtual bool handleEvent(const sf::Event event) override;

	virtual void handleInput(const input::InputManager &input) override;

	virtual void update(const TimeSpan deltaTime) override;
	virtual void updateFrequent(const TimeSpan deltaTime) override;

	virtual void renderApplication(sf::RenderTarget &renderTarget, const engine::window::WindowView &view) override;
	virtual void renderInterface(sf::RenderTarget &renderTarget, const engine::window::WindowView &view) override;

	void setDisplayMode(const DisplayMode mode);

protected:
	void imageChanged(SharedPointer<image::Image> image);
	lang::SignalBind imageChangedBind;

	void filelistChanged(SizeType numFiles);
	lang::SignalBind filelistChangedBind;

	void screenResized(const math::VC2U &size);
	lang::SignalBind screenResizedBind;

	void filesDropped(const std::vector<engine::window::DroppedFile> &files);
	lang::SignalBind filesDroppedBind;

	resource::FontResource *font = nullptr;

	float framePadding = 20.f;

	enum class DisplayRotation { Top, Right, Bottom, Left };
	struct DisplayRotationInfo
	{
		DisplayRotation rotation;
		math::FloatDamper visualRotation;
	} rotationInfo;
	math::VC2 transformSizeByDisplayRotation(const math::VC2 &vec) const;
	
	SteadyTimer elapsedTimer;

	SteadyTimer clickTimer;
	SteadyTimer changeTimer;

	void updateDefaultScale();
	bool updateImageInfo();

	void drawLoaderGadget(sf::RenderTarget &renderTarget,
		const math::VC2 &centerPosition, float width = 12.f, float size = 5.f);

	resource::ShaderResource *backgroundShader = nullptr;
	resource::ShaderResource *gaussianShader = nullptr;
	resource::TextureResource *topBarTexture = nullptr;

	struct CurrentState
	{
		SharedPointer<image::Image> image;

		bool hasData = false;
		bool hasError = false;
		image::ImageData data;

		TimeSpan frameTime;

		// Current dir display things
		SizeType indexInDirectory = 0;
		SizeType numImagesInDirectory = 0;
		String currentDirname;
	};
	CurrentState current;

	void updateEventNotifications(TimeSpan delta);
	void drawEventNotifications(sf::RenderTarget &target, const engine::window::WindowView &view, const math::VC2 &drawOffset);
	void addEventNotification(String text, math::COL color = math::COL::white);

	struct EventNotification
	{
		String text;
		math::COL color;
		Time expiry;
		float offset;
	};
	std::vector<EventNotification> eventNotifications;
	const TimeSpan eventNotificationDuration = TimeSpan::fromMilliseconds(2500);

	math::FloatDamper defaultScale;
	math::FloatDamper imageScale;
	math::VC2Damper positionOffset;

	SteadyTimer frameTimer;

	void updateViewport(const engine::window::WindowView &view);
	void enforceOversizeLimits(float scale, bool enforceTarget = true);
	math::VC2 positionOversizeLimit;
	math::FloatRect viewport;

	math::VC2 calculateMouseDiff(const engine::window::WindowView &view, 
		const math::VC2 &mousePos, float currentScale, float targetScale);

	DisplayMode displayMode = Normal;
	DisplayMode pendingDisplayMode = Normal;

	float dragged = 0.f;
	bool deleteWasReleased = true;

	bool borderlessFullscreen = false;

	//////////////////////

	struct ViewerInfoVisible
	{
		bool top : 1;
		bool bottom : 1;
	};
	ViewerInfoVisible viewerInfoVisible;

	struct ViewerInfoDampers
	{
		math::FloatDamper top;
		math::FloatDamper bottom;
	};
	ViewerInfoDampers viewerInfo;

	bool displaySmooth = true;

	math::VC2I lastMousePosition;

	bool showManagerStatus = false;
	bool showSchedulerStatus = false;

	engine::window::WindowManager *windowManager = nullptr;
	viewer::ViewerManager *viewerManager = nullptr;
};

TS_END_PACKAGE2()
