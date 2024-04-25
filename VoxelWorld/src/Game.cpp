#include "Game.h"
#include <iostream>
#include <fstream>
#include "Core/ResourceManager.h"
#include "Renderer/Camera.h"
#include "Renderer/VoxelMesh.h"

#define OGT_VOX_IMPLEMENTATION
#include "ogt_vox.h"
#include "Renderer/VoxelTexture.h"

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
	Update();
	Draw();
	SDL_GL_SwapWindow(Application::SDLWindow);

	return !ShouldQuit;
}
glm::vec2 MousePosition;


std::shared_ptr<VoxelMesh> voxelMesh;
std::shared_ptr<VoxelTexture> voxelTexture;
unsigned int _texture = 0;
Camera camera;


void Game::Init()
{
	ResourceManager& rm = ResourceManager::GetInstance();
	rm.LoadShader("res/Shaders/voxel.shader", "Quad");

	Application::SetMousePosition(MousePosition.x, MousePosition.y);
	Application::HideCursor();

	voxelMesh = VoxelMesh::Create(glm::vec3(100));

	std::ifstream t("res/skeleton.vox", std::ios::binary);
	assert(t.is_open());
	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	uint8_t* buffer = new uint8_t[size];
	t.seekg(0);
	t.read((char*) & buffer[0], size);
	t.close();
	const ogt_vox_scene* scene = ogt_vox_read_scene(buffer, size);
	delete[] buffer;

	const int GRID_SIZE = scene->models[0]->size_x;
	std::vector<glm::vec4> voxels(GRID_SIZE * GRID_SIZE * GRID_SIZE);

	for (int x = 0; x < GRID_SIZE; x++)
	{
		for (int y = 0; y < GRID_SIZE; y++)
		{
			for (int z = 0; z < GRID_SIZE; z++)
			{
				int index = scene->models[0]->voxel_data[x * GRID_SIZE * GRID_SIZE + y * GRID_SIZE + z];
				//voxels[x * GRID_SIZE * GRID_SIZE + y * GRID_SIZE + z] = glm::vec4(glm::vec3((rand() % 256) / 255.0f, (rand() % 256) / 255.0f, (rand() % 256) / 255.0f), 1.0f);
				if (index > 0)
				{
					float r = scene->palette.color[index].r / 255.f;
					float g = scene->palette.color[index].g / 255.f;
					float b = scene->palette.color[index].b / 255.f;
					float a = scene->palette.color[index].a / 255.f;
					voxels[x * GRID_SIZE * GRID_SIZE + y * GRID_SIZE + z] = glm::vec4(r, g, b, a);
				}
			
				//std::cout << col.r << std::endl;
			}
		}
	}
	voxelTexture = VoxelTexture::Create(glm::ivec3(GRID_SIZE), voxels.data());
	ogt_vox_destroy_scene(scene);

	camera = Camera(75.0f, Application::W_WIDTH, Application::W_HEIGHT);

}

unsigned int quadVAO = 0;
unsigned int quadVBO;

void Game::Draw()
{

	// render image to quad
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ResourceManager::GetInstance().GetShader("Quad")->Bind();
	voxelMesh->VA->Bind();
	voxelMesh->IB->Bind();
	voxelTexture->Bind();

	ResourceManager::GetInstance().GetShader("Quad")->SetVec3("camera_position", camera.GetPosition());
	ResourceManager::GetInstance().GetShader("Quad")->SetMat4("camera_view", camera.GetViewMatrix());
	ResourceManager::GetInstance().GetShader("Quad")->SetMat4("camera_projection", camera.GetProjectionMatrix());
	ResourceManager::GetInstance().GetShader("Quad")->SetVec3("grid_size", voxelTexture->Size);

	glDrawElements(GL_TRIANGLES, voxelMesh->Indices.size(), GL_UNSIGNED_INT, 0);
}

void Game::Update()
{
	std::cout << camera.GetPosition().x << " " << camera.GetPosition().y << " " << camera.GetPosition().z <<  std::endl;
	double last_mouse_x, last_mouse_y;
	last_mouse_x = Application::Input.GetMousePosition().x;
	last_mouse_y = Application::Input.GetMousePosition().y;


	// Calculate mouse movement delta
	double delta_x = last_mouse_x - MousePosition.x;
	double delta_y = MousePosition.y - last_mouse_y;  // Inverted y-axis


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
	glm::vec3 up = camera.GetUpVector();
	const float speed = 0.1f;
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


	if (Application::Input.KeyPressed(SDL_SCANCODE_ESCAPE))
	{
		ShouldQuit = true;
	}

	MousePosition = glm::vec2(1280/2, 720/2);
	Application::SetMousePosition(MousePosition.x, MousePosition.y);
	
}
