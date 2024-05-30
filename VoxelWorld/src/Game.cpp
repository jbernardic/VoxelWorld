#include "Game.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include "Core/ResourceManager.h"
#include "Scene/Camera.h"
#include <glm/glm.hpp>

#define OGT_VOX_IMPLEMENTATION
#include "ogt_vox.h"
#include "Scene/Mesh.h"
#include "Import/MeshImport.h"
#include <glm/gtx/transform.hpp>

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

std::shared_ptr<Mesh> mesh;

void Game::Init()
{
	mesh = Import::LoadMeshes("res/skeleton.glb")[0];
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
}

void Game::Draw()
{
	auto& ctx = Application::Vulkan.DrawContext;
	ctx.surfaces.clear();
	for (const auto& s : mesh->Surfaces)
	{
		RenderMeshInfo r;
		r.firstIndex = s.firstIndex;
		r.indexCount = s.indexCount;
		r.indexBuffer = mesh->Buffers.indexBuffer.buffer;
		r.vertexBufferAddress = mesh->Buffers.vertexBufferAddress;

		glm::mat4 view = glm::translate(glm::vec3{ 0,3,-5 });
		glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)Application::Vulkan.DrawExtent.width / (float)Application::Vulkan.DrawExtent.height, 0.1f, 10000.0f);
		projection[1][1] *= 1;

		glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		r.transform = projection * view * model;
		ctx.surfaces.emplace_back(r);
	}
}


void Game::Update(double deltaTime)
{	
}
