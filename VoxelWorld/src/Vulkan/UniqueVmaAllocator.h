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
	// Move constructor
	AllocatedImage(AllocatedImage&& t) noexcept
		: image(t.image),
		imageView(std::move(t.imageView)),
		allocation(t.allocation),
		imageExtent(t.imageExtent),
		imageFormat(t.imageFormat)
	{
		t.image = nullptr;
		t.allocation = nullptr;
	}

	// Default constructor
	AllocatedImage()
		: image(nullptr),
		imageView(nullptr),
		allocation(nullptr),
		imageExtent({ 0, 0, 0 }),
		imageFormat(vk::Format::eUndefined)
	{}

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
			vmaDestroyImage(ptr, e.image, e.allocation);
		}
		for (const auto& e : buffers)
		{
			vmaDestroyBuffer(ptr, e.buffer, e.allocation);
		}
		vmaDestroyAllocator(ptr);
	}
	void Set(VmaAllocator allocator)
	{
		this->ptr = allocator;
	}
	AllocatedImage* AddImage(AllocatedImage&& image)
	{
		images.push_back(std::move(image));
		return &images.back();
	}
	AllocatedBuffer* AddBuffer(AllocatedBuffer&& buffer)
	{
		buffers.push_back(std::move(buffer));
		return &buffers.back();
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
	AllocatedBuffer* CreateBufferUnique(size_t allocSize, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		return AddBuffer(std::move(CreateBuffer(allocSize, usage, memoryUsage)));
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
	std::deque<AllocatedImage> images;
	std::deque<AllocatedBuffer> buffers;
};