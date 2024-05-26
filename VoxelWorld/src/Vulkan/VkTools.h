#pragma once
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <fstream>

#include "VkImages.h"

namespace vk {
    namespace tool {
        constexpr VkPhysicalDeviceDynamicRenderingFeaturesKHR DynamicRenderingFeature{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .dynamicRendering = VK_TRUE,
        };

        constexpr VkPhysicalDeviceSynchronization2FeaturesKHR Sync2Feature{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
            .synchronization2 = VK_TRUE,
        };

        constexpr VkPhysicalDeviceBufferDeviceAddressFeatures BufferDeviceAddressFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
            .bufferDeviceAddress = VK_TRUE
        };

        inline void TransitionImage(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout)
        {
            vk::ImageMemoryBarrier2 imageBarrier;
            imageBarrier.setSrcStageMask(vk::PipelineStageFlagBits2::eAllCommands);
            imageBarrier.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite);
            imageBarrier.setDstStageMask(vk::PipelineStageFlagBits2::eAllCommands);
            imageBarrier.setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead);

            imageBarrier.setOldLayout(currentLayout);
            imageBarrier.setNewLayout(newLayout);

            vk::ImageAspectFlags aspectMask = (newLayout == vk::ImageLayout::eDepthAttachmentOptimal) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
            vk::ImageSubresourceRange imageSubresourceRange(aspectMask, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS);
            imageBarrier.setSubresourceRange(imageSubresourceRange);
            imageBarrier.setImage(image);

            vk::DependencyInfo depInfo;
            depInfo.setImageMemoryBarrierCount(1);
            depInfo.setPImageMemoryBarriers(&imageBarrier);

            cmd.pipelineBarrier2(depInfo);
        }

        inline void CheckLayers(std::vector<char const*> const& layers, std::vector<vk::LayerProperties> const& properties)
        {
            bool check = std::all_of(layers.begin(),
                layers.end(),
                [&properties](char const* name)
            {
                return std::any_of(properties.begin(),
                properties.end(),
                [&name](vk::LayerProperties const& property) { return strcmp(property.layerName, name) == 0; });
            });
            if (!check)
            {
                throw std::runtime_error("Set the environment variable VK_LAYER_PATH to point to the location of your layers");
            }
        }

        inline void CheckExtensions(const std::vector<vk::ExtensionProperties>& extensions, const std::vector<const char*>& properties)
        {
            for (const char* name : properties)
            {
                bool found = false;
                for (const auto& ext : extensions)
                {
                    if (strcmp(ext.extensionName, name) == 0)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    throw std::runtime_error("Extension " + std::string(name) + " not supported.");
                }
            }
        }

        //QUEUE FAMILY----

        struct QueueFamilyIndices
        {
            uint32_t Main;
            uint32_t AsyncCompute;
            uint32_t AsyncTransfer;
        };

        inline QueueFamilyIndices FindQueueFamilies(vk::PhysicalDevice device)
        {
            std::vector<vk::QueueFamilyProperties> queueFamilyProperties = device.getQueueFamilyProperties();

            //Select specialized families
            std::pair<vk::QueueFlags, uint32_t> main;
            std::pair<vk::QueueFlags, uint32_t> asyncCompute;
            std::pair<vk::QueueFlags, uint32_t> asyncTransfer;

            for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
            {
                const auto& prop = queueFamilyProperties[i];
                if (prop.queueFlags & vk::QueueFlagBits::eGraphics)
                {
                    main = std::max(main, std::make_pair(prop.queueFlags, i));
                }
                if (prop.queueFlags & vk::QueueFlagBits::eCompute)
                {
                    if (!asyncCompute.first)
                    {
                        asyncCompute = std::make_pair(prop.queueFlags, i);
                    }
                    else asyncCompute = std::min(asyncCompute, std::make_pair(prop.queueFlags, i));
                }
                if (prop.queueFlags & vk::QueueFlagBits::eTransfer)
                {
                    if (!asyncTransfer.first)
                    {
                        asyncTransfer = std::make_pair(prop.queueFlags, i);
                    }
                    else asyncTransfer = std::min(asyncTransfer, std::make_pair(prop.queueFlags, i));
                }
            }
            if (!main.first || !asyncCompute.first || !asyncTransfer.first)
            {
                throw std::runtime_error("Error: The GPU does not support required graphics, compute, or transfer operations.");
            }

            QueueFamilyIndices res{};
            res.Main = main.second;
            res.AsyncCompute = asyncCompute.second;
            res.AsyncTransfer = asyncTransfer.second;
            return res;
        }

        inline uint32_t EvaluatePhysicalDevice(vk::PhysicalDevice device)
        {
            uint32_t score = 0;
            vk::PhysicalDeviceProperties props = device.getProperties();

            if (props.apiVersion < VK_API_VERSION_1_3)
            {
                return 0;
            }

            if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            {
                score += 1000; // Discrete GPUs are preferred
            }
            else if (props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
            {
                score += 500;
            }

            score += props.limits.maxImageDimension2D;

            //Check for dedicated VRAM size
            if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            {
                if (props.limits.bufferImageGranularity >= (256 * 1024)) // Assuming 256 MB minimum VRAM
                {
                    score += 200; // Additional score for dedicated VRAM size
                }
            }

            return score;
        }

        inline vk::PhysicalDevice ChoosePhysicalDevice(std::vector<vk::PhysicalDevice> physicalDevices)
        {
            if (physicalDevices.size() == 0)
            {
                throw std::runtime_error("failed to find GPUs with Vulkan support!");
            }

            std::sort(physicalDevices.begin(), physicalDevices.end(), [&](const vk::PhysicalDevice& device1, const vk::PhysicalDevice& device2)
            {
                return EvaluatePhysicalDevice(device1) > EvaluatePhysicalDevice(device2);
            });
            return physicalDevices[0];
        }

        inline vk::Extent2D ChooseSwapExtent(uint32_t desired_width, uint32_t desired_height, const vk::SurfaceCapabilitiesKHR& capabilities)
        {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                return capabilities.currentExtent;
            }
            else
            {
                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(desired_width),
                    static_cast<uint32_t>(desired_height)
                };

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                return actualExtent;
            }
        }

        inline vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
        {
            for (const auto& availableFormat : availableFormats)
            {
                if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                {
                    return availableFormat;
                }
            }
            return availableFormats[0];
        }
        inline vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
        {
            for (const auto& availablePresentMode : availablePresentModes)
            {
                if (availablePresentMode == vk::PresentModeKHR::eMailbox)
                {
                    return availablePresentMode;
                }
            }
            return vk::PresentModeKHR::eFifo;
        }

        inline std::vector<char> ReadFileBinary(const std::string& filename)
        {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            if (!file.is_open())
            {
                throw std::runtime_error("failed to open file!");
            }

            size_t fileSize = (size_t)file.tellg();
            std::vector<char> buffer(fileSize);
            file.seekg(0);
            file.read(buffer.data(), fileSize);
            file.close();
            return buffer;
        }

        inline vk::ShaderModuleCreateInfo ShaderModuleCreateInfo(std::vector<char>& code)
        {
            vk::ShaderModuleCreateInfo createInfo{};
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
            return createInfo;
        }
        inline vk::PipelineShaderStageCreateInfo ShaderStageCreateInfo(vk::ShaderModule shaderModule, vk::ShaderStageFlagBits stage, const char* name)
        {
            vk::PipelineShaderStageCreateInfo shaderStageInfo{};
            shaderStageInfo.stage = stage;
            shaderStageInfo.module = shaderModule;
            shaderStageInfo.pName = name;
            return shaderStageInfo;
        }
    }
}