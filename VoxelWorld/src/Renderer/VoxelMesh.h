#pragma once
#include <glm/glm.hpp>
#include <array>
#include <memory>
#include "../Graphics/VertexArray.h"
#include "../Graphics/Texture.h"

class VoxelMesh
{
public:
	static std::shared_ptr<VoxelMesh> Create(glm::ivec3 size, const uint8_t* data, const glm::vec4* palette);

	void UpdateVoxels(glm::ivec3 offset, glm::ivec3 size, const uint8_t* data);
	uint8_t GetVoxel(glm::ivec3 position) const;

	VoxelMesh(std::array<glm::vec3, 8> vertices, std::array<int, 36> indices, glm::ivec3 size, const uint8_t* data);
	const std::array<glm::vec3, 8> Vertices;
	const std::array<int, 36> Indices;
	const glm::ivec3 Size;
	std::shared_ptr<VertexArray> VA;
	std::shared_ptr<Buffer> VB;
	std::shared_ptr<Buffer> IB;
	std::shared_ptr<Texture<uint8_t>> VoxelTexture;
	std::shared_ptr<Texture<glm::vec4>> PaletteTexture;
	std::shared_ptr<Texture<uint8_t>> OpacityMap;

private:
	std::vector<uint8_t> data;
};