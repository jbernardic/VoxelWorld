#pragma once
#include "../Asset/ModelAsset.h"
#include "Camera.h"
#include "Common.h"
#include "../Animations/Skeleton.h"

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
		skeleton = std::unique_ptr<Skeleton>(new Skeleton(*c.skeleton));
	}
	Math::Transform transform;
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::unique_ptr<Skeleton> skeleton;
	uint32_t texture = 0;
};


class Scene
{
public:
	Model* LoadModel(ModelAsset&& meshAsset);
	Model* CopyModel(const Model& model);
	void Render(const Camera& camera);
private:
	std::vector<std::unique_ptr<Model>> models;
	std::vector<Texture> textures;
};