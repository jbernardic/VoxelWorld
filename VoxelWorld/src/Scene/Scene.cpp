#include "Scene.h"
#include "../Core/Application.h"


Model* Scene::LoadModel(ModelAsset&& modelAsset)
{
	Model* model = new Model();
	if (!modelAsset.Skelet.joints.empty())
	{
		model->skeleton = LoadSkeleton(std::move(modelAsset.Skelet));
		if (!modelAsset.Animations.empty())
		{
			model->animator = std::move(AnimationController(model->skeleton.get(), std::move(modelAsset.Animations)));
		}
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
				meshInfo.pushConstants.jointMatrixBuffer = Application::Vulkan.GetBufferAddress(*model->skeleton->jointMatrixBuffer);
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


std::unique_ptr<Skeleton> Scene::LoadSkeleton(ModelAsset::Skeleton&& s)
{
	Skeleton* skeleton = new Skeleton();
	skeleton->joints = std::move(s.joints);
	skeleton->nodes = std::move(s.nodes);
	std::vector<glm::mat4> jointMatrices;
	jointMatrices.reserve(skeleton->joints.size());

	for (const auto& jointNode : skeleton->joints)
	{
		auto& bone = skeleton->nodes[jointNode];
		jointMatrices.push_back(bone.globalTransform * bone.inverseBindMatrix);
	}

	skeleton->jointMatrixBuffer = Application::Vulkan.UploadJointMatrices(jointMatrices);
	return std::unique_ptr<Skeleton>(skeleton);
}

void Scene::UpdateSkeletonBuffer(Skeleton& skeleton)
{
	std::vector<glm::mat4> jointMatrices;
	jointMatrices.reserve(skeleton.joints.size());

	for (const auto& jointNode : skeleton.joints)
	{
		auto& bone = skeleton.nodes[jointNode];
		jointMatrices.push_back(bone.globalTransform * bone.inverseBindMatrix);
	}

	//std::cout << skeleton.joints[0].transform.translation.x << std::endl;

	Application::Vulkan.UpdateJointMatrices(skeleton.jointMatrixBuffer, jointMatrices);
}

Model* Scene::CopyModel(const Model& model)
{
	models.push_back(std::unique_ptr<Model>(new Model(model)));
	return models.back().get();
}

void Scene::Render(const Camera& camera)
{
	auto& ctx = Application::Vulkan.DrawContext;
	for (std::unique_ptr<Model>& model : models)
	{
		model->animator->Play();
		UpdateSkeletonBuffer(*model->skeleton);

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
