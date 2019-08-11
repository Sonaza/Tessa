#include "Precompiled.h"
#include "ts/ivie/scenes/ImageViewerScene.h"

#include "ts/tessa/file/ArchivistFilesystem.h"
#include "ts/tessa/file/InputFile.h"
#include "ts/tessa/file/FileUtils.h"
#include "ts/tessa/file/FileListW.h"

#include "FreeImage.h"

#include "ts/ivie/util/NaturalSort.h"
#include "ts/ivie/viewer/FileScanner.h"

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
	return true;
}

void ImageViewerScene::stop()
{
}

void ImageViewerScene::loadImage()
{
	std::wstring wpath;
	application->getCommando().getNthParameter(0, wpath);

	std::wstring dir = file::utils::getDirname(wpath);
	if (dir.empty())
		dir = file::utils::getWorkingDirectoryWide();

	viewer::FileScanner &fileScanner = getGigaton<viewer::FileScanner>();
	std::vector<std::wstring> filelist = fileScanner.getFileList();

	file::InputFile file;
	file.open(wpath, file::InputFileMode_ReadBinary);

	BigSizeType size = file.getSize();
	char *buffer = new char[size];
	file.read(buffer, size);
	file.close();

	FIMEMORY *memory = FreeImage_OpenMemory((BYTE*)buffer, (DWORD)size);

	FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeU(wpath.c_str());
	if (format == FIF_UNKNOWN)
	{
		// Try to get file type from file name instead
		format = FreeImage_GetFIFFromFilenameU(wpath.c_str());
	}

	FIBITMAP* bitmap = FreeImage_LoadFromMemory(format, memory);
	if (!bitmap)
	{
		TS_PRINTF("FreeImage_LoadFromMemory failed\n");

		FreeImage_CloseMemory(memory);
		delete[] buffer;

		return;
	}

	if (FreeImage_GetBPP(bitmap) != 32)
	{
		FIBITMAP* temp = FreeImage_ConvertTo32Bits(bitmap);

		// Unload previous image and replace it with the new one
		FreeImage_Unload(bitmap);
		bitmap = temp;
	}

	BYTE* bits = FreeImage_GetBits(bitmap);
	if (!bits)
	{
		TS_PRINTF("FreeImage_GetBits returned null\n");

		FreeImage_CloseMemory(memory);
		delete[] buffer;

		return;
	}

	uint32_t width = FreeImage_GetWidth(bitmap);
	uint32_t height = FreeImage_GetHeight(bitmap);

	sf::Image image;
	image.create(width, height, bits);

	moireTexture.loadFromImage(image);
	moireTexture.setSmooth(true);
	moireTexture.generateMipmap();

	FreeImage_Unload(bitmap);

	FreeImage_CloseMemory(memory);
	delete[] buffer;
}

void ImageViewerScene::loadResources(resource::ResourceManager &rm)
{
	loadImage();

	sprite.setTexture(moireTexture);

	{
		texSize = moireTexture.getSize();
		float scale = 1000.f / (float)texSize.y;
		sprite.setScale(scale, scale);

		system::WindowManager &wm = application->getManager<system::WindowManager>();
		math::VC2U windowSize = wm.getSize();
		sprite.setPosition(windowSize.x / 2.f, windowSize.y / 2.f);

		sprite.setOrigin(texSize.x / 2.f, texSize.y / 2.f);
	}

	blurTexture.create(texSize.x, texSize.y);

	shader.loadFromFile("convert.glsl", sf::Shader::Fragment);
}

bool ImageViewerScene::handleEvent(const sf::Event event)
{
	switch (event.type)
	{
		case sf::Event::KeyPressed:
		{
			if (event.key.code == sf::Keyboard::Left)
			{
// 				swapTexture(-1);
				return true;
			}
			if (event.key.code == sf::Keyboard::Right)
			{
// 				swapTexture(1);
				return true;
			}
		}
		break;
	}

	return false;
}

/*
void ImageViewerScene::swapTexture(Int32 dir)
{
	currentIndex += dir;
	if (currentIndex < 0)
		currentIndex = (Int32)(textures.size() - 1);
	if (currentIndex >= textures.size())
		currentIndex = 0;

	resource::TextureResource *texture = textures[currentIndex];

	if (texture && texture->isLoaded())
	{
		sf::Texture *tex = texture->getResource();
		sprite.setTexture(*tex, true);

		math::VC2U size = tex->getSize();
		float scale = 800.f / (float)size.y;

		sprite.setScale(scale, scale);

		system::WindowManager &wm = application->getManager<system::WindowManager>();
		math::VC2U windowSize = wm.getSize();
		sprite.setPosition(windowSize.x / 2.f, windowSize.y / 2.f);

		sprite.setOrigin(size.x / 2.f, size.y / 2.f);
	}
}*/

void ImageViewerScene::update(const TimeSpan deltaTime)
{
	system::WindowManager &wm = application->getManager<system::WindowManager>();
	math::VC2U windowSize = wm.getSize();
	float offset = sf::Mouse::getPosition(wm.getRenderWindow()).x / (float)windowSize.x;

	float scale = (900.f + offset * 600.f) / (float)texSize.y;
	sprite.setScale(scale, scale);
}

void ImageViewerScene::render(sf::RenderWindow &renderWindow)
{
	sf::RenderStates states;
	states.shader = &shader;
	renderWindow.draw(sprite, states);
}

TS_END_PACKAGE2()
