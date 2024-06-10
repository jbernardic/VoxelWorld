#pragma once
#include "../Scene/Common.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include "../Animation/Common.h"

class ModelAsset
{
public:
	const static glm::uint ErrorColor = 4294902015; //magenta

	struct Surface
	{
		uint32_t firstIndex;
		uint32_t indexCount;
	};

	struct Mesh
	{
		std::vector<uint32_t> Indices;
		std::vector<Vertex> Vertices;
		std::vector<Surface> Surfaces;
		std::vector<VertexBone> VertexBones;
	};

	struct Image
	{
		unsigned char* data;
		vk::Format format;
		vk::Extent3D size;
		bool error = false;
		~Image();
	};

	struct Skeleton
	{
		std::vector<Node> nodes;
		std::vector<uint32_t> joints;
	};

	Skeleton Skelet;
	std::vector<Mesh> Meshes;
	std::vector<std::unique_ptr<Image>> Textures;
	std::unordered_map<std::string, Animation> Animations;
};

namespace Asset {
	ModelAsset LoadModelGLTF(std::filesystem::path filePath);
}