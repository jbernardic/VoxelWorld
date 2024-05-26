#pragma once
#include "VkTypes.h"

namespace vk {
	namespace tool {
		inline void CopyImageToImage(vk::CommandBuffer cmd, vk::Image source, vk::Image destination, vk::Extent2D srcSize, vk::Extent2D dstSize)
		{
			vk::ImageBlit2 blitRegion;
			blitRegion.sType = vk::StructureType::eImageBlit2;
			blitRegion.pNext = nullptr;

			blitRegion.srcOffsets[1].setX(srcSize.width);
			blitRegion.srcOffsets[1].setY(srcSize.height);
			blitRegion.srcOffsets[1].setZ(1);

			blitRegion.dstOffsets[1].setX(dstSize.width);
			blitRegion.dstOffsets[1].setY(dstSize.height);
			blitRegion.dstOffsets[1].setZ(1);

			blitRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blitRegion.srcSubresource.baseArrayLayer = 0;
			blitRegion.srcSubresource.layerCount = 1;
			blitRegion.srcSubresource.mipLevel = 0;

			blitRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			blitRegion.dstSubresource.baseArrayLayer = 0;
			blitRegion.dstSubresource.layerCount = 1;
			blitRegion.dstSubresource.mipLevel = 0;

			vk::BlitImageInfo2 blitInfo;
			blitInfo.sType = vk::StructureType::eBlitImageInfo2;
			blitInfo.pNext = nullptr;
			blitInfo.dstImage = destination;
			blitInfo.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
			blitInfo.srcImage = source;
			blitInfo.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
			blitInfo.filter = vk::Filter::eLinear;
			blitInfo.regionCount = 1;
			blitInfo.pRegions = &blitRegion;

			cmd.blitImage2(blitInfo);
		}

		inline vk::ImageCreateInfo ImageCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent3D extent)
		{
			vk::ImageCreateInfo info{};
			info.setImageType(vk::ImageType::e2D)
				.setFormat(format)
				.setExtent(extent)
				.setMipLevels(1)
				.setArrayLayers(1)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setTiling(vk::ImageTiling::eOptimal)
				.setUsage(usageFlags);

			return info;
		}

		inline vk::ImageViewCreateInfo ImageViewCreateInfo(vk::Format format, vk::Image image, vk::ImageAspectFlags aspectFlags)
		{
			vk::ImageViewCreateInfo info{};
			info.setViewType(vk::ImageViewType::e2D)
				.setImage(image)
				.setFormat(format)
				.setSubresourceRange(vk::ImageSubresourceRange()
					.setAspectMask(aspectFlags)
					.setBaseMipLevel(0)
					.setLevelCount(1)
					.setBaseArrayLayer(0)
					.setLayerCount(1));

			return info;
		}

		inline vk::RenderingAttachmentInfo AttachmentInfo(vk::ImageView view, vk::ClearValue* clear, vk::ImageLayout layout)
		{
			vk::RenderingAttachmentInfo colorAttachment{};
			colorAttachment.imageView = view;
			colorAttachment.imageLayout = layout;
			colorAttachment.loadOp = clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
			colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
			if (clear)
			{
				colorAttachment.clearValue = *clear;
			}

			return colorAttachment;
		}
	}
}