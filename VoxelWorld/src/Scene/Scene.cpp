#include "Scene.h"
#include <vector>


std::list<Model>::iterator Scene::LoadModel(const ModelAsset& modelAsset)
{
	Model model;

	//initialize joint matrices
	std::vector<glm::mat4> jointMatrices;
	jointMatrices.reserve(modelAsset.Skeleton.size());

	for (const auto& bone : modelAsset.Skeleton)
	{
		SkeletonJoint joint;
		joint.children = std::move(bone.children);
		joint.inverseBindMatrix = std::move(bone.inverseBindMatrix);
		joint.parent = bone.parent;
		joint.transform = Math::Transform(bone.localTransform);
		model.skeleton.joints.push_back(joint);
		jointMatrices.push_back(bone.globalTransform * bone.inverseBindMatrix);
	}
	const auto& joints = Application::Vulkan.UploadJointMatrices(jointMatrices);

	for (const auto& meshAsset : modelAsset.Meshes)
	{
		Mesh mesh;
		mesh.buffers = Application::Vulkan.UploadMesh(meshAsset.Indices, meshAsset.Vertices, meshAsset.VertexBones);
		for (const auto& surface : meshAsset.Surfaces)
		{
			RenderMeshInfo meshInfo;
			meshInfo.firstIndex = surface.firstIndex;
			meshInfo.indexCount = surface.indexCount;
			meshInfo.indexBuffer = mesh.buffers.indexBuffer.buffer;

			meshInfo.pushConstants.jointMatrixBuffer = Application::Vulkan.GetBufferAddress(joints);
			meshInfo.pushConstants.vertexBuffer = Application::Vulkan.GetBufferAddress(mesh.buffers.vertexBuffer);
			meshInfo.pushConstants.vertexBoneBuffer = Application::Vulkan.GetBufferAddress(mesh.buffers.vertexBoneBuffer);

			//TODO: change later

			glm::mat4 view = glm::translate(glm::vec3{ 0,0, -400 });
			glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)Application::Vulkan.DrawExtent.width / (float)Application::Vulkan.DrawExtent.height, 0.1f, 10000.0f);
			projection[1][1] *= -1;

			meshInfo.pushConstants.worldMatrix = projection*view;
			mesh.surfaces.push_back(std::move(meshInfo));
		}
		model.meshes.push_back(std::move(mesh));
	}
	models.push_back(std::move(model));
	return --models.end();
}

//void Scene::DestroyMesh(std::list<MeshReference>::iterator it)
//{
//	//TODO: destroy skeleton
//	Application::Vulkan.Allocator.DestroyBuffer(it->meshBuffers.indexBuffer);
//	Application::Vulkan.Allocator.DestroyBuffer(it->meshBuffers.vertexBuffer);
//	loadedMeshes.erase(it);
//}

void Scene::Render()
{
	auto& ctx = Application::Vulkan.DrawContext;
	for (auto& model : models)
	{
		for (Mesh& mesh : model.meshes)
		{
			for (auto& surface : mesh.surfaces)
			{
				ctx.meshes.push_back(surface);
			}
		}
	}
}
