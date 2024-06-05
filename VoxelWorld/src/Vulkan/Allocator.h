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
	vk::ImageView imageView;
	VmaAllocation allocation;
	vk::Extent3D imageExtent;
	vk::Format imageFormat;
};

class Allocator
{
public:
	template <class T>
	class Accessor
	{
		public:
			Accessor() = default;
			Accessor(uint32_t index, std::vector<T>* items) : index(index), items(items) {};

			const T& operator*() const
			{
				return items->at(index);
			}

			const T* operator->() const
			{
				return &items->at(index);
			}

			bool IsValid()
			{
				return items->at(index).allocation != nullptr;
			}

			uint32_t GetIndex()
			{
				return index;
			}
			
		private:
			uint32_t index = 0;
			std::vector<T>* items = nullptr;
	};

	Allocator(vk::PhysicalDevice pdevice, vk::Device device, vk::Instance instance)
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = pdevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		vmaCreateAllocator(&allocatorInfo, &allocator);
	}

	~Allocator()
	{
		for (const auto& e : images)
		{
			if (e.image != VK_NULL_HANDLE)
			{
				vmaDestroyImage(allocator, e.image, e.allocation);
			}
			VmaAllocatorInfo info;
			vmaGetAllocatorInfo(allocator, &info);
			vkDestroyImageView(info.device, e.imageView, nullptr);
		}
		for (const auto& e : buffers)
		{
			if (e.buffer != VK_NULL_HANDLE)
			{
				vmaDestroyBuffer(allocator, e.buffer, e.allocation);
			}
		}
		vmaDestroyAllocator(allocator);
	}
	VmaAllocator GetAllocator() const
	{
		return allocator;
	}
	void DestroyBuffer(const AllocatedBuffer& buffer)
	{
		vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
	}
	void DestroyImage(const AllocatedImage& image)
	{
		vmaDestroyImage(allocator, image.image, image.allocation);
	}
	Accessor<AllocatedBuffer> CreateBuffer(size_t allocSize, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage)
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
		VkResult res = vmaCreateBuffer(allocator, (VkBufferCreateInfo*)&bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation,
			&newBuffer.info);

		return addBuffer(std::move(newBuffer));
	}

	Accessor<AllocatedImage> CreateImage(vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage)
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
		vmaCreateImage(allocator, (VkImageCreateInfo*)&img_info, &allocinfo, (VkImage*)&newImage.image, &newImage.allocation, nullptr);

		// if the format is a depth format, we will need to have it use the correct aspect flag
		vk::ImageAspectFlags aspectFlag = vk::ImageAspectFlagBits::eColor;
		if (format == vk::Format::eD32Sfloat)
		{
			aspectFlag = vk::ImageAspectFlagBits::eDepth;
		}

		// build a image-view for the image
		vk::ImageViewCreateInfo view_info = vk::tool::ImageViewCreateInfo(format, newImage.image, aspectFlag);
		view_info.subresourceRange.levelCount = img_info.mipLevels;

		VmaAllocatorInfo info;
		vmaGetAllocatorInfo(allocator, &info);

		vkCreateImageView(info.device, (VkImageViewCreateInfo*) & view_info, nullptr, (VkImageView*) & newImage.imageView);

		return addImage(std::move(newImage));
	}

	//void DestroyBuffer(Accessor<AllocatedBuffer>& buffer)
	//{
	//	freeBufferIndices.push(buffer.GetIndex());
	//	vmaDestroyBuffer(allocator, buffer->buffer, buffer->allocation);
	//}

	//void DestroyImage(Accessor<AllocatedImage>& image)
	//{
	//	freeImageIndices.push(image.GetIndex());
	//	vmaDestroyImage(allocator, image->image, image->allocation);
	//}

private:

	Accessor<AllocatedImage> addImage(AllocatedImage&& image)
	{
		uint32_t index;
		if (freeImageIndices.empty())
		{
			images.push_back(std::move(image));
			index = images.size() - 1;
		}
		else
		{
			images.insert(images.begin() + freeImageIndices.top(), std::move(image));
			index = freeImageIndices.top();
			freeImageIndices.pop();
		}
		return Accessor<AllocatedImage>(index, &images);
	}

	Accessor<AllocatedBuffer> addBuffer(AllocatedBuffer&& buffer)
	{
		uint32_t index;
		if (freeBufferIndices.empty())
		{
			buffers.push_back(std::move(buffer));
			index = buffers.size() - 1;
		}
		else
		{
			buffers.insert(buffers.begin() + freeBufferIndices.top(), std::move(buffer));
			index = freeBufferIndices.top();
			freeBufferIndices.pop();
		}
		return Accessor<AllocatedBuffer>(index, &buffers);
	}

	VmaAllocator allocator;
	std::stack<uint32_t> freeImageIndices;
	std::stack<uint32_t> freeBufferIndices;
	std::vector<AllocatedImage> images;
	std::vector<AllocatedBuffer> buffers;
};