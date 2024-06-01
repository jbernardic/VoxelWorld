#pragma once
#include <iostream>
#include "../Asset/ModelAsset.h"

struct MeshReference
{
	RenderMeshInfo meshInfo;
	MeshBuffers meshBuffers;
};

class Scene
{
public:
	std::vector<std::list<MeshReference>::iterator> LoadModel(const ModelAsset& meshAsset);
	void DestroyMesh(std::list<MeshReference>::iterator it);
	void Render();
private:
	std::list<MeshReference> loadedMeshes;
};