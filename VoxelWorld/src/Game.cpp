#include "Game.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include "Core/ResourceManager.h"
#include "Renderer/Camera.h"
#include <glm/glm.hpp>

#define OGT_VOX_IMPLEMENTATION
#include "ogt_vox.h"

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
	Application::Vulkan.Draw();
	return !shouldQuit;
}

void Game::Init()
{
}

void Game::Draw()
{
}


void Game::Update(double deltaTime)
{	
}
