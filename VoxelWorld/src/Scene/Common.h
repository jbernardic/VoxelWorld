#pragma once
#include "../Helpers/Math.h"
#include "../Vulkan/VkTypes.h"

struct Node
{
	std::optional<uint32_t> parent;
	std::vector<uint32_t> children;
	Math::Transform transform;
	glm::mat4 globalTransform;
	glm::mat4 inverseBindMatrix;
};

struct Keyframe
{
	float time;
	std::vector<std::pair<uint32_t, Math::Transform>> transforms;
};

struct Animation
{
	std::string name;
	std::vector<Keyframe> keyframes;
};
