#pragma once
#include "../Core/Application.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

class MeshAsset
{
public:

	struct Surface
	{
		uint32_t firstIndex;
		uint32_t indexCount;
	};

	std::vector<uint32_t> Indices;
	std::vector<Vertex> Vertices;
	std::vector<Surface> Surfaces;
	std::string Name;
};

namespace Asset {
	std::vector<MeshAsset> LoadGLTFMeshes(std::filesystem::path filePath);
}