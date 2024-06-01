#include "Scene.h"
#include "Transform.h"
#include <vector>


std::vector<std::list<MeshReference>::iterator> Scene::LoadModel(const ModelAsset& modelAsset)
{
	std::vector<glm::mat4> jointMatrices;
	jointMatrices.reserve(modelAsset.Skeleton.size());

	for (const auto& bone : modelAsset.Skeleton)
	{
		jointMatrices.push_back(bone.globalTransform * bone.inverseBindMatrix);
	}

	std::vector<std::list<MeshReference>::iterator> iterators;

	const auto& joints = Application::Vulkan.UploadJointMatrices(jointMatrices);

	for (const auto& mesh : modelAsset.Meshes)
	{
		MeshBuffers buffers = Application::Vulkan.UploadMesh(mesh.Indices, mesh.Vertices, mesh.VertexBones);
		
		for (const auto& surface : mesh.Surfaces)
		{
			MeshReference meshRef;
			meshRef.meshBuffers = buffers;
			meshRef.meshInfo.firstIndex = surface.firstIndex;
			meshRef.meshInfo.indexCount = surface.indexCount;
			meshRef.meshInfo.indexBuffer = buffers.indexBuffer.buffer;
			meshRef.meshInfo.pushConstants.jointMatrixBuffer = joints.second;

			meshRef.meshInfo.pushConstants.vertexBuffer = buffers.vertexBufferAddress;
			meshRef.meshInfo.pushConstants.vertexBoneBuffer = buffers.vertexBoneBufferAddress;
			meshRef.meshInfo.pushConstants.worldMatrix = glm::mat4(1.0);
			loadedMeshes.push_back(meshRef);
			std::list<MeshReference>::iterator i = loadedMeshes.end();
			iterators.push_back(--i);
		}
	}

	return iterators;
}

void Scene::DestroyMesh(std::list<MeshReference>::iterator it)
{
	//TODO: destroy skeleton
	Application::Vulkan.Allocator.DestroyBuffer(it->meshBuffers.indexBuffer);
	Application::Vulkan.Allocator.DestroyBuffer(it->meshBuffers.vertexBuffer);
	loadedMeshes.erase(it);
}

void Scene::Render()
{
	auto& ctx = Application::Vulkan.DrawContext;
	for (auto& m : loadedMeshes)
	{
		glm::mat4 view = glm::translate(glm::vec3{ 0,0, -400});
		glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)Application::Vulkan.DrawExtent.width / (float)Application::Vulkan.DrawExtent.height, 0.1f, 10000.0f);
		projection[1][1] *= -1;

		m.meshInfo.pushConstants.worldMatrix = projection * view;

		ctx.meshes.push_back(m.meshInfo);
	}
}
