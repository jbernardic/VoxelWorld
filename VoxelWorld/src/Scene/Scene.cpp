#include "Scene.h"
#include "../Core/Application.h"


Model* Scene::LoadModel(ModelAsset&& modelAsset)
{
	Model* model = new Model();
	if (!modelAsset.Skeleton.empty())
	{
		model->skeleton = std::make_unique<Skeleton>(std::move(modelAsset.Skeleton));
	}

	//upload textures
	if (modelAsset.Textures.size() > 0)
	{
		VkAllocator::Accessor<AllocatedImage> texture = Application::Vulkan.UploadImage(modelAsset.Textures[0]->data, modelAsset.Textures[0]->size, modelAsset.Textures[0]->format,
			vk::ImageUsageFlagBits::eSampled);

		textures.push_back({ texture, *Application::Vulkan.DefaultSampler });
		Application::Vulkan.UpdateMeshTextures(textures);
	}

	for (const auto& meshAsset : modelAsset.Meshes)
	{
		Mesh* mesh = new Mesh();
		mesh->buffers = Application::Vulkan.UploadMesh(meshAsset.Indices, meshAsset.Vertices, meshAsset.VertexBones);
		for (const auto& surface : meshAsset.Surfaces)
		{
			RenderMeshInfo meshInfo;
			meshInfo.firstIndex = surface.firstIndex;
			meshInfo.indexCount = surface.indexCount;
			meshInfo.indexBuffer = mesh->buffers.indexBuffer->buffer;

			if (model->skeleton)
			{
				meshInfo.pushConstants.jointMatrixBuffer = Application::Vulkan.GetBufferAddress(*model->skeleton->JointMatrixBuffer);
				meshInfo.pushConstants.useSkeleton = true;
			}
			meshInfo.pushConstants.vertexBuffer = Application::Vulkan.GetBufferAddress(*mesh->buffers.vertexBuffer);
			meshInfo.pushConstants.vertexBoneBuffer = Application::Vulkan.GetBufferAddress(*mesh->buffers.vertexBoneBuffer);
			mesh->surfaces.push_back(std::move(meshInfo));
		}
		model->meshes.push_back(std::shared_ptr<Mesh>(mesh));
	}
	models.push_back(std::unique_ptr<Model>(model));
	return models.back().get();
}

Model* Scene::CopyModel(const Model& model)
{
	models.push_back(std::unique_ptr<Model>(new Model(model)));
	return models.back().get();
}

void Scene::Render(const Camera& camera)
{
	std::cout << Application::Vulkan.Allocator->GetBufferCount() << std::endl;
	auto& ctx = Application::Vulkan.DrawContext;
	for (std::unique_ptr<Model>& model : models)
	{
		for (auto& mesh : model->meshes)
		{
			for (RenderMeshInfo& surface : mesh->surfaces)
			{
				surface.pushConstants.worldMatrix = camera.GetViewProjectionMatrix();
				ctx.meshes.push_back(surface);
			}
		}
	}
}
