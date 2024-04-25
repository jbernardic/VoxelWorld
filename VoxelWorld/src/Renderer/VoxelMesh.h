#pragma once
#include <glm/glm.hpp>
#include <array>
#include <memory>
#include "../Graphics/VertexArray.h"

class VoxelMesh
{
public:
	static std::shared_ptr<VoxelMesh> Create(glm::vec3 size, glm::vec3 position = glm::vec3());

	VoxelMesh(std::array<glm::vec3, 8> vertices, std::array<int, 36> indices);
	const std::array<glm::vec3, 8> Vertices;
	const std::array<int, 36> Indices;
	std::shared_ptr<VertexArray> VA;
	std::shared_ptr<Buffer> VB;
	std::shared_ptr<Buffer> IB;
};