#pragma once
#include "VkTypes.h"
#include "VkImages.h"
#include <SDL.h>

constexpr unsigned int FRAME_OVERLAP = 2;

#ifdef NDEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif

class VkContext
{
public:

    struct FrameData
    {
        vk::UniqueSemaphore SwapchainSemaphore, RenderSemaphore;
        vk::UniqueFence RenderFence;
        vk::UniqueCommandPool CommandPool;
        vk::UniqueCommandBuffer MainCommandBuffer;
    };

    void Init(SDL_Window* window, uint32_t width, uint32_t height);
    void Draw();

    AllocatedImage* DrawImage;
    vk::Extent2D DrawExtent;
    vk::UniqueInstance Instance;
    vk::UniqueDebugUtilsMessengerEXT DebugMessenger;
    vk::PhysicalDevice PhysicalDevice;
    vk::UniqueDevice Device;
    vk::UniqueSurfaceKHR Surface;
    vk::UniqueSwapchainKHR Swapchain;
    vk::SwapchainCreateInfoKHR SwapchainInfo;
    std::vector<vk::Image> SwapchainImages;
    std::vector<vk::UniqueImageView> SwapchainImageViews;
    vk::UniquePipelineLayout GraphicsPipelineLayout;
    vk::UniquePipeline GraphicsPipeline;
    UniqueVmaAllocator Allocator;
    FrameData Frames[FRAME_OVERLAP];
    uint32_t FrameNumber;
    vk::Queue GraphicsQueue;
    FrameData& GetCurrentFrame() { return Frames[FrameNumber % FRAME_OVERLAP]; }

private:
    SDL_Window* sdl_window;
    uint32_t w_width;
    uint32_t w_height;
    void init_vulkan();
    void init_swapchain();
    void init_commands();
    void init_sync_structures();
    void init_graphics_pipeline();
    void create_swapchain(vk::Extent2D extent, vk::SurfaceFormatKHR surfaceFormat, vk::PresentModeKHR presentMode, vk::SurfaceTransformFlagBitsKHR transfrom, uint32_t imageCount);
    void draw_background(vk::CommandBuffer cmd);
    void draw_geometry(vk::CommandBuffer cmd);
};