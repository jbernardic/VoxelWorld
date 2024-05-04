#include "Game.h"
#include <iostream>
#include <fstream>
#include <chrono>
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
	long long timeDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - time).count();
	time = std::chrono::high_resolution_clock::now();
	Update(timeDelta/ 1000000000.0);
	Draw();
	SDL_GL_SwapWindow(Application::SDLWindow);
	return !shouldQuit;
}
glm::vec2 MousePosition;


std::shared_ptr<VoxelMesh> voxelMesh;
std::shared_ptr<Buffer> normalMapBuffer;
std::shared_ptr<Buffer> AOMapBuffer;
std::shared_ptr<Buffer> voxelUniformBuffer;
int GRID_SIZE;
unsigned int _texture = 0;
Camera camera;


void Game::Init()
{
	ResourceManager& rm = ResourceManager::GetInstance();
	rm.LoadShader("res/Shaders/voxel_light.shader", "voxel_light");
	rm.LoadShader("res/Shaders/voxel_normals.shader", "voxel_normals");

	Application::SetMousePosition(MousePosition.x, MousePosition.y);
	Application::HideCursor();

	std::ifstream t("res/sphere2.vox", std::ios::binary);
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

	std::vector<uint8_t> empty(GRID_SIZE * GRID_SIZE * GRID_SIZE, 1.0);
	voxelMesh = VoxelMesh::Create(glm::vec3(GRID_SIZE), scene->models[0]->voxel_data, palette.data());
	ogt_vox_destroy_scene(scene);

	camera = Camera(75.0f, Application::W_WIDTH, Application::W_HEIGHT);

	size_t _size = GRID_SIZE * GRID_SIZE * GRID_SIZE;
	normalMapBuffer = Buffer::Create(Buffer::Type::ShaderStorageBuffer, _size*(sizeof(glm::vec3)+sizeof(float)), nullptr);

	struct voxelUB
	{
		glm::vec4 camera_position;
		glm::vec4 sun_direction = glm::vec4(0.0, -1.0, -1.0, -1.0);
		glm::ivec4 grid_size = glm::ivec4(GRID_SIZE);
	};
	voxelUB ub;
	voxelUniformBuffer = Buffer::Create(Buffer::Type::UniformBuffer, sizeof(voxelUB), &ub);
	normalMapBuffer->BindBase(0);
	voxelUniformBuffer->BindBase(1);
	//AOMapBuffer->BindBase(2);

	voxelMesh->VoxelTexture->Bind(0);
	ResourceManager::GetInstance().GetShader("voxel_normals")->Bind();
	ResourceManager::GetInstance().GetShader("voxel_normals")->SetVec3("work_position", glm::vec3(GRID_SIZE)/glm::vec3(2.0));
	glDispatchCompute(GRID_SIZE / 8, GRID_SIZE / 8, GRID_SIZE / 8);

	

}

unsigned int quadVAO = 0;
unsigned int quadVBO;

void Game::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	ResourceManager::GetInstance().GetShader("voxel_light")->Bind();
	voxelMesh->VA->Bind();
	voxelMesh->IB->Bind();
	voxelMesh->VoxelTexture->Bind(0);
	voxelMesh->PaletteTexture->Bind(1);
	voxelMesh->OpacityMap->Bind(2);

	ResourceManager::GetInstance().GetShader("voxel_light")->SetMat4("camera_view", camera.GetViewMatrix());
	ResourceManager::GetInstance().GetShader("voxel_light")->SetMat4("camera_projection", camera.GetProjectionMatrix());
	glm::vec3 camPos = camera.GetPosition();
	voxelUniformBuffer->SubData(sizeof(glm::vec3), &camPos, 0);

	glDrawElements(GL_TRIANGLES, voxelMesh->Indices.size(), GL_UNSIGNED_INT, 0);
}


void Game::Update(double deltaTime)
{
	//std::cout << 1.0/deltaTime <<  std::endl;
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
	const float speed = 100.0*deltaTime;
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
		shouldQuit = true;
	}
	if (Application::Input.MouseButtonDown(SDL_BUTTON_LEFT))
	{
		int data_size = 10;
		glm::ivec3 mapPos = glm::ivec3(camera.GetPosition() - glm::vec3(data_size/2) + camera.GetForwardVector()*glm::vec3(10.0));
		if(!(mapPos.x < 0 or mapPos.x+9 >= GRID_SIZE or mapPos.y < 0 or mapPos.y+9 >= GRID_SIZE or mapPos.z < 0 or mapPos.z+9 >= GRID_SIZE))
		{
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
			//change later
			voxelMesh->UpdateVoxels(mapPos, glm::ivec3(data_size), new_data.data());
			ResourceManager::GetInstance().GetShader("voxel_normals")->Bind();
			ResourceManager::GetInstance().GetShader("voxel_normals")->SetVec3("work_position", camera.GetPosition());
			glDispatchCompute(64 / 8, 64 / 8, 64 / 8);
		}

	}

	MousePosition = glm::vec2(1280/2, 720/2);
	Application::SetMousePosition(MousePosition.x, MousePosition.y);
	
}
