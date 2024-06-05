#pragma once
#include "VkTypes.h"
#include "VkTools.h"

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
			if (e.image != VK_NULL_HANDLE)
			{
				vmaDestroyImage(ptr, e.image, e.allocation);
			}
		}
		for (const auto& e : buffers)
		{
			if (e.buffer != VK_NULL_HANDLE)
			{
				vmaDestroyBuffer(ptr, e.buffer, e.allocation);
			}
		}
		vmaDestroyAllocator(ptr);
	}
	void Set(VmaAllocator allocator)
	{
		this->ptr = allocator;
	}
	//void DestroyBuffer(const AllocatedBuffer& buffer)
	//{
	//	vmaDestroyBuffer(ptr, buffer.buffer, buffer.allocation);
	//}
	//void DestroyImage(const AllocatedImage& image)
	//{
	//	vmaDestroyImage(ptr, image.image, image.allocation);
	//}
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
	std::list<AllocatedBuffer>::const_iterator CreateBufferUnique(size_t allocSize, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		auto buffer = CreateBuffer(allocSize, usage, memoryUsage);
		buffers.push_back(std::move(buffer));
		return --buffers.end();
	}

	AllocatedImage CreateImage(vk::Device device, vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage)
	{
		AllocatedImage newImage;
		newImage.imageFormat = format;
		newImage.imageExtent = size;

		vk::ImageCreateInfo img_info = vk::tool::ImageCreateInfo(format, usage, size);

		// always allocate images on dedicated GPU memory
		VmaAllocationCreateInfo allocinfo = {};
		allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// allocate and create the image
		vmaCreateImage(ptr, (VkImageCreateInfo*)&img_info, &allocinfo, (VkImage*)&newImage.image, &newImage.allocation, nullptr);

		// if the format is a depth format, we will need to have it use the correct
		// aspect flag
		vk::ImageAspectFlags aspectFlag = vk::ImageAspectFlagBits::eColor;
		if (format == vk::Format::eD32Sfloat)
		{
			aspectFlag = vk::ImageAspectFlagBits::eDepth;
		}

		// build a image-view for the image
		vk::ImageViewCreateInfo view_info = vk::tool::ImageViewCreateInfo(format, newImage.image, aspectFlag);
		view_info.subresourceRange.levelCount = img_info.mipLevels;

		newImage.imageView = device.createImageViewUnique(view_info);
		return newImage;
	}
	std::list<AllocatedImage>::const_iterator CreateImageUnique(vk::Device device, vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage)
	{
		auto image = CreateImage(device, size, format, usage);
		images.push_back(std::move(image));
		return --images.end();
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
	std::list<AllocatedImage> images;
	std::list<AllocatedBuffer> buffers;
};