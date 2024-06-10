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

		float a = -1.0f;
		std::vector<AnimationKeyframe>::iterator prevKeyframe;
		if (nextKeyframe != sampler.keyframes.begin())
		{
			prevKeyframe = nextKeyframe-1;
			a = (time - prevKeyframe->time) / (nextKeyframe->time - prevKeyframe->time);
		}

		for (auto& channel : sampler.channels)
		{
			Node& node = skeleton->nodes[channel.nodeIndex];
			if (channel.path == fastgltf::AnimationPath::Translation)
			{
				glm::vec4 transform = nextKeyframe->transform;
				if (a >= 0.0f) transform = glm::mix(prevKeyframe->transform, transform, a);
				node.transform.translation = transform;
			}
			else if (channel.path == fastgltf::AnimationPath::Rotation)
			{
				glm::quat q;
				q.x = nextKeyframe->transform.x;
				q.y = nextKeyframe->transform.y;
				q.z = nextKeyframe->transform.z;
				q.w = nextKeyframe->transform.w;
				if (a >= 0.0f)
				{
					glm::quat pq;
					pq.x = nextKeyframe->transform.x;
					pq.y = nextKeyframe->transform.y;
					pq.z = nextKeyframe->transform.z;
					pq.w = nextKeyframe->transform.w;

					q = glm::normalize(glm::slerp(pq, q, a));
				}
				node.transform.rotation = q;
			}
			else if (channel.path == fastgltf::AnimationPath::Scale)
			{
				glm::vec4 transform = nextKeyframe->transform;
				if (a >= 0.0f) transform = glm::mix(prevKeyframe->transform, transform, a);
				node.transform.scale = transform;
			}
		}
	}

	skeleton->calculateGlobalTransforms();

	time += 0.002f;
	if (time > getCurrentAnimation().duration)
	{
		Reset();
	}
}

void AnimationController::Reset()
{
	time = 0.0f;
}


