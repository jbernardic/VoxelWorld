#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>
#include <deque>

#include <vulkan/vulkan.hpp>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <SDL.h>
#include <SDL_vulkan.h>

#include "UniqueVmaAllocator.h"

struct Vertex
{

	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
};

struct VertexBone
{
	glm::vec4 joint;
	glm::vec4 weight;
};

// holds the resources needed for a mesh
struct MeshBuffers
{

	std::list<AllocatedBuffer>::const_iterator indexBuffer;
	std::list<AllocatedBuffer>::const_iterator vertexBuffer;
	std::list<AllocatedBuffer>::const_iterator vertexBoneBuffer;
};

// push constants for our mesh object draws
struct MeshPushConstants
{
	glm::mat4 worldMatrix;
	float useSkeleton;
	VkDeviceAddress vertexBuffer;
	VkDeviceAddress vertexBoneBuffer;
	VkDeviceAddress jointMatrixBuffer;
};

struct RenderMeshInfo
{
	uint32_t indexCount;
	uint32_t firstIndex;
	
	vk::Buffer indexBuffer;
	MeshPushConstants pushConstants;
};

struct Texture
{
	std::list<AllocatedImage>::const_iterator image;
	vk::Sampler sampler;
};