#include "AnimationController.h"

AnimationController::AnimationController(Skeleton* skeleton, std::unordered_map<std::string, Animation>&& animations)
	: skeleton(skeleton), animations(std::move(animations))
{
	current_animation = this->animations.begin()->second.name;
}

void AnimationController::ChangeAnimation(std::string& name)
{
	current_animation = name;
	Reset();
}

Animation& AnimationController::getCurrentAnimation()
{
	return animations[current_animation];
}

void AnimationController::Play()
{
	for (auto& sampler : getCurrentAnimation().samplers)
	{
		auto nextKeyframe = std::lower_bound(sampler.keyframes.begin(), sampler.keyframes.end(), time, 
			[](AnimationKeyframe& key, float _time) {
				return key.time < _time;
			});

		glm::vec3 transform = nextKeyframe->transform;
		if (nextKeyframe != sampler.keyframes.begin())
		{
			auto prevKeyframe = nextKeyframe - 1;
			float interpolationValue = (time - prevKeyframe->time) / (nextKeyframe->time - prevKeyframe->time);
			transform = prevKeyframe->transform + interpolationValue * (nextKeyframe->transform - prevKeyframe->transform);
		}
		for (auto& channel : sampler.channels)
		{
			Node& node = skeleton->joints[channel.nodeIndex];
			if (channel.path == fastgltf::AnimationPath::Translation)
			{
				node.transform.translation = transform;
			}
			else if (channel.path == fastgltf::AnimationPath::Rotation)
			{
				node.transform.rotation = transform;
			}
			else if (channel.path == fastgltf::AnimationPath::Scale)
			{
				node.transform.scale = transform;
			}
		}
	}

	skeleton->calculateGlobalTransforms();

	time += 0.001f;
	if (time > getCurrentAnimation().duration)
	{
		Reset();
	}
}

void AnimationController::Reset()
{
	time = 0.0f;
}


