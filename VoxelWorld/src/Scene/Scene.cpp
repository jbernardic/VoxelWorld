#include "Scene.h"
#include "Transform.h"


void Scene::LoadMeshAssets(const std::vector<MeshAsset>& meshAssets)
{
	for (const auto& meshAsset : meshAssets)
	{
		GPUMeshBuffers buffers = Application::Vulkan.UploadMesh(meshAsset.Indices, meshAsset.Vertices);
		for (const auto& surface : meshAsset.Surfaces)
		{
			MeshReference meshRef;
			meshRef.meshBuffers = buffers;
			meshRef.meshInfo.firstIndex = surface.firstIndex;
			meshRef.meshInfo.indexCount = surface.indexCount;
			meshRef.meshInfo.indexBuffer = buffers.indexBuffer.buffer;
			meshRef.meshInfo.vertexBufferAddress = buffers.vertexBufferAddress;
			meshRef.meshInfo.transform = glm::mat4(1.0);
			meshRef.name = meshAsset.Name;
			loadedMeshes[meshRef.name] = meshRef;
		}
	}
}

void Scene::DestroyMesh(const char* name)
{
	auto it = loadedMeshes.find(name);
	Application::Vulkan.Allocator.DestroyBuffer(it->second.meshBuffers.indexBuffer);
	Application::Vulkan.Allocator.DestroyBuffer(it->second.meshBuffers.vertexBuffer);
	loadedMeshes.erase(it);
}

void Scene::Render()
{
	auto& ctx = Application::Vulkan.DrawContext;
	for (auto& m : loadedMeshes)
	{
		glm::mat4 view = glm::translate(glm::vec3{ 0,3,-5 });
		glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)Application::Vulkan.DrawExtent.width / (float)Application::Vulkan.DrawExtent.height, 0.1f, 10000.0f);
		projection[1][1] *= 1;

		glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		m.second.meshInfo.transform = projection * view * model;

		ctx.surfaces.emplace_back(m.second.meshInfo);
	}
}
