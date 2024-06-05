#pragma once
#include "VkTypes.h"

constexpr unsigned int FRAME_OVERLAP = 2;
constexpr unsigned int MAX_TEXTURE_COUNT = 1000;

#ifdef NDEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif

struct DrawContext
{
    std::vector<RenderMeshInfo> meshes;
    void clear()
    {
        meshes.clear();
    }
};

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
    void ImmediateSubmit(std::function<void(vk::CommandBuffer cmd)>&& function);
    FrameData& GetCurrentFrame() { return Frames[FrameNumber % FRAME_OVERLAP]; }
    vk::Extent2D GetDrawExtent() { return vk::Extent2D(DrawImage->imageExtent.width, DrawImage->imageExtent.height); }
    
    VkDeviceAddress GetBufferAddress(const AllocatedBuffer& buffer);

    MeshBuffers UploadMesh(const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices, const std::vector<VertexBone>& bones);
    std::list<AllocatedImage>::const_iterator UploadImage(void* data, vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage);
    std::list<AllocatedBuffer>::const_iterator UploadJointMatrices(const std::vector<glm::mat4>& mats);

    void UpdateMeshTextures(const std::vector<Texture>& textures);

    vk::UniqueInstance Instance;
    vk::UniqueDebugUtilsMessengerEXT DebugMessenger;
    vk::PhysicalDevice PhysicalDevice;
    vk::UniqueDevice Device;
    UniqueVmaAllocator Allocator;
    vk::UniqueSurfaceKHR Surface;
    vk::UniqueSwapchainKHR Swapchain;
    vk::SwapchainCreateInfoKHR SwapchainInfo;
    std::vector<vk::Image> SwapchainImages;
    std::vector<vk::UniqueImageView> SwapchainImageViews;
    vk::UniquePipelineLayout GraphicsPipelineLayout;
    vk::UniquePipeline GraphicsPipeline;

    FrameData Frames[FRAME_OVERLAP];
    uint32_t FrameNumber;
    vk::Queue GraphicsQueue;

    vk::UniqueFence ImmFence;
    vk::UniqueCommandPool ImmCommandPool;
    vk::UniqueCommandBuffer ImmCommandBuffer;
    std::list<AllocatedImage>::const_iterator DrawImage;
    DrawContext DrawContext;
    
    vk::UniqueSampler DefaultSampler;

    vk::UniqueDescriptorPool DescriptorPool;
    vk::DescriptorSet MeshDescriptorSet;
    vk::UniqueDescriptorSetLayout MeshDescriptorSetLayout;

private:
    SDL_Window* sdl_window;
    uint32_t w_width;
    uint32_t w_height;
    void init_vulkan();
    void init_swapchain();
    void init_commands();
    void init_sync_structures();
    void init_graphics_pipeline();
    void init_descriptors();
    void init_samplers();
    void create_swapchain(vk::Extent2D extent, vk::SurfaceFormatKHR surfaceFormat, vk::PresentModeKHR presentMode, vk::SurfaceTransformFlagBitsKHR transfrom, uint32_t imageCount);
    void draw_background(vk::CommandBuffer cmd);
    void draw_geometry(vk::CommandBuffer cmd);
};