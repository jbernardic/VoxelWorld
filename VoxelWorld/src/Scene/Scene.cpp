#include "Scene.h"
#include <vector>


std::list<Model>::iterator Scene::LoadModel(const ModelAsset& modelAsset)
{
	Model model;
	if (!modelAsset.Skeleton.empty())
	{
		//initialize joint matrices
		std::vector<glm::mat4> jointMatrices;
		jointMatrices.reserve(modelAsset.Skeleton.size());

		for (const auto& bone : modelAsset.Skeleton)
		{
			SkeletonJoint joint;
			joint.children = bone.children;
			joint.inverseBindMatrix = bone.inverseBindMatrix;
			joint.parent = bone.parent;
			joint.transform = Math::Transform(bone.localTransform);
			jointMatrices.push_back(bone.globalTransform * bone.inverseBindMatrix);
		}
		model.skeleton.jointMatrixBuffer = Application::Vulkan.UploadJointMatrices(jointMatrices);
	}

	//upload textures
	if (modelAsset.Textures.size() > 0)
	{
		Allocator::Accessor<AllocatedImage> texture = Application::Vulkan.UploadImage(modelAsset.Textures[0]->data, modelAsset.Textures[0]->size, modelAsset.Textures[0]->format,
			vk::ImageUsageFlagBits::eSampled);

		textures.push_back({ texture, *Application::Vulkan.DefaultSampler });
		Application::Vulkan.UpdateMeshTextures(textures);
	}

	for (const auto& meshAsset : modelAsset.Meshes)
	{
		Mesh mesh;
		mesh.buffers = Application::Vulkan.UploadMesh(meshAsset.Indices, meshAsset.Vertices, meshAsset.VertexBones);
		for (const auto& surface : meshAsset.Surfaces)
		{
			RenderMeshInfo meshInfo;
			meshInfo.firstIndex = surface.firstIndex;
			meshInfo.indexCount = surface.indexCount;
			meshInfo.indexBuffer = mesh.buffers.indexBuffer->buffer;

			if(!modelAsset.Skeleton.empty())
				meshInfo.pushConstants.jointMatrixBuffer = Application::Vulkan.GetBufferAddress(*model.skeleton.jointMatrixBuffer);
			meshInfo.pushConstants.useSkeleton = modelAsset.Skeleton.size();
			meshInfo.pushConstants.vertexBuffer = Application::Vulkan.GetBufferAddress(*mesh.buffers.vertexBuffer);
			meshInfo.pushConstants.vertexBoneBuffer = Application::Vulkan.GetBufferAddress(*mesh.buffers.vertexBoneBuffer);
			mesh.surfaces.push_back(std::move(meshInfo));
		}
		model.meshes.push_back(std::move(mesh));
	}
	models.push_back(std::move(model));
	return --models.end();
}

std::list<Model>::iterator Scene::LoadModel(const Model& model)
{
	models.push_back(model);
	return std::list<Model>::iterator();
}

//void Scene::DestroyMesh(std::list<MeshReference>::iterator it)
//{
//	//TODO: destroy skeleton
//	Application::Vulkan.Allocator.DestroyBuffer(it->meshBuffers.indexBuffer);
//	Application::Vulkan.Allocator.DestroyBuffer(it->meshBuffers.vertexBuffer);
//	loadedMeshes.erase(it);
//}

void Scene::Render(const Camera& camera)
{
	auto& ctx = Application::Vulkan.DrawContext;
	for (auto& model : models)
	{
		for (Mesh& mesh : model.meshes)
		{
			for (auto& surface : mesh.surfaces)
			{
				surface.pushConstants.worldMatrix = camera.GetViewProjectionMatrix();
				ctx.meshes.push_back(surface);
			}
		}
	}
}
