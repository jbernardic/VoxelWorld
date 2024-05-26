#pragma once
#include "VkTypes.h"

struct AllocatedBuffer
{
	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo info;
};

struct AllocatedImage
{
	vk::Image image;
	vk::UniqueImageView imageView;
	VmaAllocation allocation;
	vk::Extent3D imageExtent;
	vk::Format imageFormat;
};

class UniqueVmaAllocator
{
public:
	~UniqueVmaAllocator()
	{
		for (const auto& e : images)
		{
			vmaDestroyImage(ptr, e.first, e.second);
		}
		for (const auto& e : buffers)
		{
			vmaDestroyBuffer(ptr, e.first, e.second);
		}
		vmaDestroyAllocator(ptr);
	}
	void Set(VmaAllocator allocator)
	{
		this->ptr = allocator;
	}
	void RegisterImage(AllocatedImage& image)
	{
		images.push_back({image.image, image.allocation});
	}
	void RegisterBuffer(AllocatedBuffer& buffer)
	{
		buffers.push_back({ buffer.buffer, buffer.allocation });
	}
	AllocatedBuffer CreateBuffer(size_t allocSize, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		// allocate buffer
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.size = allocSize;
		bufferInfo.usage = usage;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = memoryUsage;
		vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		AllocatedBuffer newBuffer;

		// allocate the buffer
		VkResult res = vmaCreateBuffer(ptr, (VkBufferCreateInfo*)&bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation,
			&newBuffer.info);

		return newBuffer;
	}
	AllocatedBuffer CreateBufferUnique(size_t allocSize, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		auto buffer = CreateBuffer(allocSize, usage, memoryUsage);
		RegisterBuffer(buffer);
		return buffer;
	}
	VmaAllocator Get() const
	{
		return ptr;
	}
	VmaAllocator operator*()
	{
		return ptr;
	}
	VmaAllocator operator->()
	{
		return ptr;
	}
private:
	VmaAllocator ptr;
	std::vector<std::pair<vk::Image, VmaAllocation>> images;
	std::vector<std::pair<vk::Buffer, VmaAllocation>> buffers;
};