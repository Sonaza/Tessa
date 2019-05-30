#include "Precompiled.h"
#include "ts/tessa/system/Application.h"

#include "ts/tessa/system/SceneBase.h"
#include "ts/tessa/resource/ResourceManager.h"

TS_PACKAGE1(system)

Application::Application()
{
}

Application::~Application()
{
}

void Application::initialize()
{
	initializeManagers();
	initializeWindow();
}

void Application::deinitialize()
{
	renderWindow.close();

	if (currentScene != nullptr)
		currentScene->internalStop();

	pendingScene.reset();
	currentScene.reset();
}

void Application::launch()
{
	initialize();

	if (sceneInitialize())
	{
		mainloop();
	}
	else
	{
		TS_PRINTF("Scene initialize failed!\n");
	}

	deinitialize();
}

void Application::initializeManagers()
{
	resourceManager = std::make_shared<resource::ResourceManager>();
}

void Application::initializeWindow()
{
	std::string windowTitle = "Nonograms";

	uint32_t style = sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close;

	sf::ContextSettings settings;
	settings.antialiasingLevel = 2;

	renderWindow.create(sf::VideoMode(1600, 1000), windowTitle, style, settings);
	activeView = renderWindow.getView();
}

void Application::mainloop()
{
	const sf::Time fixedDeltaTime = sf::milliseconds(16);
	sf::Time deltaAccumulator;

	sf::Clock loopTimer;
	while (applicationRunning)
	{
		if (pendingScene != nullptr)
		{
			bool successfulStart = pendingScene->internalStart();
			if (!successfulStart)
			{
				TS_PRINTF("Pending scene start failed. Scene will not be loaded.");
				pendingScene.reset();
			}

			if (currentScene != nullptr)
			{
				currentScene->internalStop();
				currentScene.reset();
			}

			currentScene = std::move(pendingScene);
			currentScene->loadResources(resourceManager);
		}

		renderWindow.clear();

		handleEvents();

		sf::Time deltaTime = updateClock.restart();
		deltaAccumulator += deltaTime;

		while (deltaAccumulator >= fixedDeltaTime)
		{
			handleUpdate(fixedDeltaTime);
			deltaAccumulator -= fixedDeltaTime;
		}

		handleRendering();

		renderWindow.display();

		sf::sleep(fixedDeltaTime - loopTimer.restart());
	}

	if (currentScene != nullptr)
	{
		currentScene->stop();
	}
}

void Application::handleEvents()
{
	sf::Event event;
	while (renderWindow.pollEvent(event))
	{
		if (currentScene != nullptr)
		{
			// If event is handled by scene the rest can be skipped
			if (currentScene->handleEvent(event))
				continue;
		}

		switch (event.type)
		{
			case sf::Event::Closed:
			{
				applicationRunning = false;
			}
			break;

			case sf::Event::KeyPressed:
			{
				if (event.key.code == sf::Keyboard::Escape)
				{
					applicationRunning = false;
				}
			}
			break;

			case sf::Event::Resized:
			{
				activeView = renderWindow.getView();
				activeView.setSize((float)event.size.width, (float)event.size.height);
				renderWindow.setView(activeView);
			}
			break;
		}
	}
}

void Application::handleUpdate(const sf::Time deltaTime)
{
	if (currentScene != nullptr)
	{
		currentScene->update(deltaTime);
	}
}

void Application::handleRendering()
{
	if (currentScene != nullptr)
	{
		currentScene->render(renderWindow);
	}
}

TS_END_PACKAGE1()
