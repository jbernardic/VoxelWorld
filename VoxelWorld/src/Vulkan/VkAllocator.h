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

class VkAllocator
{
public:
	template <class T>
	class Accessor
	{
	public:
		Accessor() = default;
		Accessor(uint32_t index, VkAllocator* allocator) : index(index), allocator(allocator) {};

		T& operator*() const
		{
			return getItems()->at(index);
		}

		T* operator->() const
		{
			return &getItems()->at(index);
		}

		bool IsValid()
		{
			return allocator != nullptr && index < getItems()->size() && getItems()->at(index).allocation != nullptr;
		}

		void Destroy()
		{
			allocator->Destroy(*this);
		}

		uint32_t GetIndex()
		{
			return index;
		}

	private:

		std::vector<T>* getItems() const
		{
			return nullptr;
		}

		uint32_t index = 0;
		VkAllocator* allocator = nullptr;
	};

	VkAllocator(vk::PhysicalDevice pdevice, vk::Device device, vk::Instance instance)
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = pdevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
	}

	~VkAllocator()
	{
		while (!freeBufferIndices.empty())
		{
			buffers[freeBufferIndices.top()] = {};
			freeBufferIndices.pop();
		}

		while (!freeImageIndices.empty())
		{
			images[freeImageIndices.top()] = {};
			freeImageIndices.pop();
		}

		for (const auto& e : images)
		{
			if (e.allocation != nullptr)
			{
				vmaDestroyImage(vmaAllocator, e.image, e.allocation);
				VmaAllocatorInfo info;
				vmaGetAllocatorInfo(vmaAllocator, &info);
				vkDestroyImageView(info.device, e.imageView, nullptr);
			}
		}
		for (const auto& e : buffers)
		{
			if (e.allocation != nullptr)
			{
				vmaDestroyBuffer(vmaAllocator, e.buffer, e.allocation);
			}
		}
		vmaDestroyAllocator(vmaAllocator);
	}

	VmaAllocator GetAllocator() const
	{
		return vmaAllocator;
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
		VkResult res = vmaCreateBuffer(vmaAllocator, (VkBufferCreateInfo*)&bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation,
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
		vmaCreateImage(vmaAllocator, (VkImageCreateInfo*)&img_info, &allocinfo, (VkImage*)&newImage.image, &newImage.allocation, nullptr);

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
		vmaGetAllocatorInfo(vmaAllocator, &info);

		vkCreateImageView(info.device, (VkImageViewCreateInfo*) & view_info, nullptr, (VkImageView*) & newImage.imageView);

		return addImage(std::move(newImage));
	}
	
	size_t GetBufferCount() { return buffers.size() - freeBufferIndices.size(); }
	size_t GetImageCount() { return images.size() - freeImageIndices.size(); }

	void Destroy(Accessor<AllocatedBuffer>& buffer)
	{
		freeBufferIndices.push(buffer.GetIndex());
		vmaDestroyBuffer(vmaAllocator, buffer->buffer, buffer->allocation);
	}

	void Destroy(Accessor<AllocatedImage>& image)
	{
		freeImageIndices.push(image.GetIndex());
		vmaDestroyImage(vmaAllocator, image->image, image->allocation);
	}

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
			index = freeImageIndices.top();
			images[index] = std::move(image);
			freeImageIndices.pop();
		}
		return Accessor<AllocatedImage>(index, this);
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
			index = freeBufferIndices.top();
			buffers[index] = std::move(buffer);
			freeBufferIndices.pop();
		}
		return Accessor<AllocatedBuffer>(index, this);
	}

	VmaAllocator vmaAllocator;
	std::stack<uint32_t> freeImageIndices;
	std::stack<uint32_t> freeBufferIndices;
	std::vector<AllocatedImage> images;
	std::vector<AllocatedBuffer> buffers;
};

template<> 
inline std::vector<AllocatedImage>* VkAllocator::Accessor<AllocatedImage>::getItems() const
{
	return &allocator->images;
}

template<>
inline std::vector<AllocatedBuffer>* VkAllocator::Accessor<AllocatedBuffer>::getItems() const
{
	return &allocator->buffers;
}