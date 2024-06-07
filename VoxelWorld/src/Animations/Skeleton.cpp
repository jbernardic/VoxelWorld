#include "Skeleton.h"
#include "../Core/Application.h"

Skeleton::Skeleton(std::vector<Node>&& joints) : joints(std::move(joints))
{
	std::vector<glm::mat4> jointMatrices;
	jointMatrices.reserve(this->joints.size());

	for (const auto& bone : this->joints)
	{
		jointMatrices.push_back(bone.globalTransform * bone.inverseBindMatrix);
	}
	JointMatrixBuffer = Application::Vulkan.UploadJointMatrices(jointMatrices);
}
