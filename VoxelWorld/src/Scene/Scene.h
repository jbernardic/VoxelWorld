#pragma once
#include <iostream>
#include "../Asset/MeshAsset.h"

struct MeshReference
{
	std::string name;
	RenderMeshInfo meshInfo;
	GPUMeshBuffers meshBuffers;
};

class Scene
{
public:
	void LoadMeshAssets(const std::vector<MeshAsset>& meshAssets);
	void DestroyMesh(const char* name);
	void Render();
private:
	std::unordered_map<std::string, MeshReference> loadedMeshes;
};