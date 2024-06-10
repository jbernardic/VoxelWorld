#pragma once
#include "../Helpers/Math.h"
#include "../Vulkan/VkTypes.h"

struct Node
{
	std::optional<uint32_t> parent;
	std::vector<uint32_t> children;
	Math::Transform transform;
	glm::mat4 globalTransform = glm::mat4(1.0);
	glm::mat4 inverseBindMatrix = glm::mat4(1.0);
};

struct Skeleton
{
	VkAllocator::Accessor<AllocatedBuffer> jointMatrixBuffer;
	std::vector<Node> nodes;
	std::vector<uint32_t> joints;
	~Skeleton()
	{
		jointMatrixBuffer.Destroy();
	}
	void calculateGlobalTransforms()
	{
		uint32_t index = 0;
		for (Node& node : nodes)
		{
			if (!node.parent.has_value())
			{
				processNode(index, glm::mat4(1.0));
			}
			index++;
		}
	}
private:
	void processNode(uint32_t index, glm::mat4 transform)
	{
		nodes[index].globalTransform = transform * nodes[index].transform.ToMat4();
		
		for (auto& child : nodes[index].children)
		{
			processNode(child, nodes[index].globalTransform);
		}
	}
};