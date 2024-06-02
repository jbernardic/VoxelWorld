#include "Game.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include "Core/ResourceManager.h"
#include "Scene/Camera.h"
#include <glm/glm.hpp>

#define OGT_VOX_IMPLEMENTATION
#include "ogt_vox.h"
#include "Asset/ModelAsset.h"
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

ModelAsset modelAsset;
Scene scene;
Camera camera;
glm::vec2 MousePosition;
bool escaped = false;

void Game::Init()
{
	modelAsset = Asset::LoadModelGLTF("res/skeleton.glb");
	scene.LoadModel(modelAsset);

	Application::SetMousePosition(MousePosition.x, MousePosition.y);
	Application::HideCursor();

	camera = Camera(70.f, (float)Application::Vulkan.DrawExtent.width, (float)Application::Vulkan.DrawExtent.height);
}

void Game::Draw()
{
	scene.Render(camera);
}

void Game::Update(double deltaTime)
{
	//abstract those later
	if (Application::Input.KeyPressed(SDL_SCANCODE_ESCAPE))
	{
		escaped = !escaped;
	}
	if (!escaped)
	{
		Application::HideCursor();
	}
	else
	{
		Application::ShowCursor();
		return;
	}

	double last_mouse_x, last_mouse_y;
	last_mouse_x = Application::Input.GetMousePosition().x;
	last_mouse_y = Application::Input.GetMousePosition().y;


	// Calculate mouse movement delta
	double delta_x = last_mouse_x - MousePosition.x;
	double delta_y = last_mouse_y - MousePosition.y;


	// Calculate camera direction
	static float yaw = -90.0f;
	static float pitch = 0.0f;

	yaw += delta_x * 0.05f;
	pitch += delta_y * 0.05f;

	// Clamp pitch to avoid flipping
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	// std::cout << _camera_direction.x << " " << _camera_direction.y << " " << _camera_direction.z << std::endl;


	glm::vec3 up_vector = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 forward = camera.GetForwardVector();
	glm::vec3 right = camera.GetRightVector();
	glm::vec3 up = -camera.GetUpVector();
	const float speed = 100.0 * deltaTime;
	if (Application::Input.KeyDown(SDL_SCANCODE_A))
	{
		camera.SetPosition(camera.GetPosition() - right * speed);
	}
	if (Application::Input.KeyDown(SDL_SCANCODE_D))
	{
		camera.SetPosition(camera.GetPosition() + right * speed);
	}
	if (Application::Input.KeyDown(SDL_SCANCODE_S))
	{
		camera.SetPosition(camera.GetPosition() - forward * speed);
	}
	if (Application::Input.KeyDown(SDL_SCANCODE_W))
	{
		camera.SetPosition(camera.GetPosition() + forward * speed);
	}
	if (Application::Input.KeyDown(SDL_SCANCODE_SPACE))
	{
		camera.SetPosition(camera.GetPosition() + up * speed);
	}

	camera.SetPitch(pitch);
	camera.SetYaw(yaw);
	MousePosition = glm::vec2(1280 / 2, 720 / 2);
	Application::SetMousePosition(MousePosition.x, MousePosition.y);
}
