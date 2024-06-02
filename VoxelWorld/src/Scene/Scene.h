#pragma once
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
	std::vector<SkeletonJoint> joints;
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
};

class Scene
{
public:

	std::list<Model>::iterator LoadModel(const ModelAsset& meshAsset);
	void Render();
private:
	std::list<Model> models;
};