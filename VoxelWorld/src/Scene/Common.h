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

struct Skeleton
{
	VkAllocator::Accessor<AllocatedBuffer> jointMatrixBuffer;
	std::vector<Node> joints;
	~Skeleton()
	{
		jointMatrixBuffer.Destroy();
	}
	void calculateGlobalTransforms()
	{
		uint32_t index = 0;
		for (Node& joint : joints)
		{
			if (!joint.parent.has_value())
			{
				processNode(index++, glm::mat4(1.0));
			}
		}
	}
private:
	void processNode(uint32_t index, glm::mat4 transform)
	{
		joints[index].globalTransform = transform * joints[index].transform.ToMat4();
		
		for (auto& child : joints[index].children)
		{
			processNode(child, joints[index].globalTransform);
		}
	}
};