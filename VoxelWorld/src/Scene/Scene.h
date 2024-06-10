#pragma once
#include "../Asset/ModelAsset.h"
#include "../Animation/AnimationController.h"
#include "Camera.h"
#include "Common.h"

struct Mesh
{
	MeshBuffers buffers;
	std::vector<RenderMeshInfo> surfaces;
	~Mesh()
	{
		buffers.indexBuffer.Destroy();
		buffers.vertexBuffer.Destroy();
		buffers.vertexBoneBuffer.Destroy();
	}
};

struct Model
{
	Model() = default;
	Model(const Model& c)
	{
		transform = c.transform;
		meshes = c.meshes;
		texture = c.texture;
		animator = c.animator;
		skeleton = std::unique_ptr<Skeleton>(new Skeleton(*c.skeleton));
	}

	Math::Transform transform;
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::unique_ptr<Skeleton> skeleton;
	std::optional<AnimationController> animator;
	uint32_t texture = 0;
};


class Scene
{
public:
	Model* LoadModel(ModelAsset&& meshAsset);
	Model* CopyModel(const Model& model);
	std::unique_ptr<Skeleton> LoadSkeleton(ModelAsset::Skeleton&& skeleton);
	void UpdateSkeletonBuffer(Skeleton& skeleton);
	void Render(const Camera& camera);
private:
	std::vector<std::unique_ptr<Model>> models;
	std::vector<Texture> textures;
};