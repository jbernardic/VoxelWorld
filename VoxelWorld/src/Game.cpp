#include "Game.h"
#include <iostream>
#include <fstream>
#include "Core/ResourceManager.h"
#include "Renderer/Camera.h"
#include "Renderer/VoxelMesh.h"
#include "Graphics/Texture.h"

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
	Update();
	Draw();
	SDL_GL_SwapWindow(Application::SDLWindow);

	return !ShouldQuit;
}
glm::vec2 MousePosition;


std::shared_ptr<VoxelMesh> voxelMesh;
std::shared_ptr<Buffer> storageBuffer;
int GRID_SIZE;
unsigned int _texture = 0;
Camera camera;


void Game::Init()
{
	ResourceManager& rm = ResourceManager::GetInstance();
	rm.LoadShader("res/Shaders/voxel_light.shader", "Quad");

	Application::SetMousePosition(MousePosition.x, MousePosition.y);
	Application::HideCursor();

	std::ifstream t("res/sphere.vox", std::ios::binary);
	assert(t.is_open());
	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	uint8_t* buffer = new uint8_t[size];
	t.seekg(0);
	t.read((char*) & buffer[0], size);
	t.close();
	const ogt_vox_scene* scene = ogt_vox_read_scene(buffer, size);
	delete[] buffer;

	GRID_SIZE = scene->models[0]->size_x;
	std::vector<glm::vec4> palette;
	for (const ogt_vox_rgba& col : scene->palette.color)
	{
		palette.emplace_back(col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a / 255.0f);
	}

	std::vector<uint8_t> empty(GRID_SIZE * GRID_SIZE * GRID_SIZE);
	voxelMesh = VoxelMesh::Create(glm::vec3(GRID_SIZE), empty.data(), palette.data());

	ogt_vox_destroy_scene(scene);

	camera = Camera(75.0f, Application::W_WIDTH, Application::W_HEIGHT);

	//std::vector<float> light_map(GRID_SIZE * GRID_SIZE * GRID_SIZE, 0.0);
	//storageBuffer = Buffer::Create(Buffer::Type::ShaderStorageBuffer, light_map.size()*sizeof(float), light_map.data());
	//storageBuffer->BindBase(0);
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
	voxelMesh->DataTexture->Bind(0);
	voxelMesh->PaletteTexture->Bind(1);

	ResourceManager::GetInstance().GetShader("Quad")->SetVec3("camera_position", camera.GetPosition());
	ResourceManager::GetInstance().GetShader("Quad")->SetMat4("camera_view", camera.GetViewMatrix());
	ResourceManager::GetInstance().GetShader("Quad")->SetMat4("camera_projection", camera.GetProjectionMatrix());
	ResourceManager::GetInstance().GetShader("Quad")->SetIVec3("grid_size", glm::vec3(GRID_SIZE));

	glDrawElements(GL_TRIANGLES, voxelMesh->Indices.size(), GL_UNSIGNED_INT, 0);
}

void Game::Update()
{
	//std::cout << camera.GetPosition().x << " " << camera.GetPosition().y << " " << camera.GetPosition().z <<  std::endl;
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
	const float speed = 0.8f;
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
	if (Application::Input.MouseButtonDown(SDL_BUTTON_LEFT))
	{
		int data_size = 10;
		glm::ivec3 mapPos = glm::ivec3(camera.GetPosition() - glm::vec3(data_size/2) + camera.GetForwardVector()*glm::vec3(10.0));
		std::vector<uint8_t> new_data(data_size * data_size * data_size);
		for (int x = 0; x < 10; x++)
		{
			for (int y = 0; y < 10; y++)
			{
				for (int z = 0; z < 10; z++)
				{
					int radius = 5;
					int t = 5;
					float distance = (x - t) * (x - t) + (y - t) * (y - t) + (z - t) * (z - t);
					if (distance < radius * radius) new_data[x * 10 * 10 + y * 10 + z] = 50;
					else new_data[x + 10 * (y + 10 * z)] = voxelMesh->GetVoxel(mapPos + glm::ivec3(x, y, z));
				}
			}
		}
		voxelMesh->UpdateData(mapPos, glm::ivec3(data_size), new_data.data());
	}

	MousePosition = glm::vec2(1280/2, 720/2);
	Application::SetMousePosition(MousePosition.x, MousePosition.y);
	
}
