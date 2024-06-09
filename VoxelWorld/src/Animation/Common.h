#pragma once
#include <vector>
#include <string>
#include <fastgltf/types.hpp>
#include "../Helpers/Math.h"
#include "../Vulkan/VkTypes.h"

struct AnimationKeyframe
{
	float time;
	glm::vec3 transform;
};

struct AnimationChannel
{
	uint32_t nodeIndex;
	fastgltf::AnimationPath path;
};

struct AnimationSampler
{
	std::vector<AnimationKeyframe> keyframes;
	std::vector<AnimationChannel> channels;
};

struct Animation
{
	std::string name;
	float duration = 0.0f;
	std::vector<AnimationSampler> samplers;
};
