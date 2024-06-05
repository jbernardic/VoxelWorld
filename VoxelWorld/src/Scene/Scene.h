#pragma once
#include "Camera.h"
#include <iostream>
#include "../Asset/ModelAsset.h"
#include "../Helpers/Math.h"

struct SkeletonJoint
{
	std::optional<uint32_t> parent;
	std::vector<uint32_t> children;
	Math::Transform transform;
	glm::mat4 inverseBindMatrix;
};

struct Skeleton
{
	Allocator::Accessor<AllocatedBuffer> jointMatrixBuffer;
};

struct Mesh
{
	MeshBuffers buffers;
	std::vector<RenderMeshInfo> surfaces;
};

struct Model
{
	Math::Transform transform;
	std::vector<Mesh> meshes;
	Skeleton skeleton;
	uint32_t texture = 0;
};

class Scene
{
public:
	std::list<Model>::iterator LoadModel(const ModelAsset& meshAsset);
	std::list<Model>::iterator LoadModel(const Model& model);
	void Render(const Camera& camera);
private:
	std::list<Model> models;
	std::vector<Texture> textures;
};