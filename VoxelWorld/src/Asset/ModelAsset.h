#pragma once
#include "../Core/Application.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

class ModelAsset
{
public:
	const static glm::uint ErrorColor = 4294902015; //magenta

	struct Surface
	{
		uint32_t firstIndex;
		uint32_t indexCount;
	};

	struct Node
	{
		std::optional<uint32_t> parent;
		std::vector<uint32_t> children;
		glm::mat4 localTransform;
		glm::mat4 globalTransform;
		glm::mat4 inverseBindMatrix;
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

	std::vector<Node> Skeleton;
	std::vector<Mesh> Meshes;
	std::vector<std::unique_ptr<Image>> Textures;
};

namespace Asset {
	ModelAsset LoadModelGLTF(std::filesystem::path filePath);
}