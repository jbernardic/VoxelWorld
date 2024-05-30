#pragma once
#include "Scene.h"
#include "Transform.h"

class Model
{
public:
	Model(std::vector<std::string>& meshes);
	Transform Transform;
private:
	std::vector<std::string> meshes;
};