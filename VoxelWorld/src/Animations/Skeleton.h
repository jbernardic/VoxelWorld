#pragma once

#include "../Vulkan/VkTypes.h"
#include "../Helpers/Math.h"
#include "../Scene/Common.h"

class Skeleton
{
public:
	Skeleton() = default;
	Skeleton(const Skeleton& copy) = default;
	Skeleton(std::vector<Node>&& joints);
	~Skeleton()
	{
		JointMatrixBuffer.Destroy();
	}
	VkAllocator::Accessor<AllocatedBuffer> JointMatrixBuffer;
private:
	std::vector<Node> joints;
};