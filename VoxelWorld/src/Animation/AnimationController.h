#pragma once
#include <vector>
#include <string>
#include <chrono>
#include "Common.h"
#include "../Scene/Common.h"

class AnimationController
{
public:
	AnimationController(Skeleton* skeleton, std::unordered_map<std::string, Animation>&& animations);
	void ChangeAnimation(std::string& name);
	void Play();
	void Reset();
	float Speed = 1.0f;
private:
	Animation& getCurrentAnimation();

	float time = 0.0f;
	Skeleton* skeleton;
	std::string current_animation = "";
	std::unordered_map<std::string, Animation> animations;
};