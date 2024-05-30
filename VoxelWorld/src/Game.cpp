#include "Game.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include "Core/ResourceManager.h"
#include "Scene/Camera.h"
#include <glm/glm.hpp>

#define OGT_VOX_IMPLEMENTATION
#include "ogt_vox.h"
#include "Asset/MeshAsset.h"
#include <glm/gtx/transform.hpp>
#include "Scene/Scene.h"

Game::Game()
{
	Application::Create();
	Init();
}

Game::~Game()
{
	Application::Destroy();
}

bool Game::Tick()
{
	SDL_Event e;
	int pollEvent = SDL_PollEvent(&e);
	Application::Input.InitState();
	while (pollEvent)
	{
		Application::Input.HandleEvent(e);
		switch (e.type)
		{
		case SDL_QUIT:
			return false;
			break;
		default:
			break;
		}
		pollEvent = SDL_PollEvent(&e);
	}
	long long timeDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - time).count();
	time = std::chrono::high_resolution_clock::now();
	Update(timeDelta/ 1000000000.0);
	Game::Draw();
	Application::Vulkan.Draw();
	return !shouldQuit;
}

MeshAsset meshAsset;
Scene scene;

void Game::Init()
{
	//checkerboard image
	auto black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 1));
	auto white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
	_testImage1 = Application::Vulkan.UploadImage((void*)&black, VkExtent3D{1, 1, 1}, vk::Format::eR8G8B8A8Unorm,
		vk::ImageUsageFlagBits::eSampled);
	_testImage2 = Application::Vulkan.UploadImage((void*)&white, VkExtent3D{ 1, 1, 1 }, vk::Format::eR8G8B8A8Unorm,
		vk::ImageUsageFlagBits::eSampled);

	std::vector<std::pair<vk::ImageView, vk::Sampler>> textures;
	textures.push_back({ *_testImage1.imageView, *Application::Vulkan.DefaultSampler });
	textures.push_back({ *_testImage2.imageView, *Application::Vulkan.DefaultSampler });
	Application::Vulkan.UpdateMeshTextures(textures);

	meshAsset = Asset::LoadGLTFMeshes("res/skeleton.glb")[0];
	scene.LoadMeshAssets({ meshAsset });
}

void Game::Draw()
{
	scene.Render();
}


void Game::Update(double deltaTime)
{	
}
