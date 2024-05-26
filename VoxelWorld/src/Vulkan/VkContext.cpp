#include "VkContext.h"
#include "VkDebug.h"
#include "VkTools.h"

#include <stdexcept>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

void VkContext::Init(SDL_Window* window, uint32_t width, uint32_t height)
{
    sdl_window = window;
    w_width = width;
    w_height = height;
    init_vulkan();
    init_swapchain();
    init_commands();
    init_sync_structures();
    init_graphics_pipeline();
}

void VkContext::Draw()
{
    Device->resetFences(*GetCurrentFrame().RenderFence);
    auto swapchainImageIndex = Device->acquireNextImageKHR(*Swapchain, 1000000000, *GetCurrentFrame().SwapchainSemaphore);
    assert(swapchainImageIndex.result == vk::Result::eSuccess);

    DrawExtent.width = DrawImage->imageExtent.width;
    DrawExtent.height = DrawImage->imageExtent.height;

    vk::CommandBuffer cmd = *GetCurrentFrame().MainCommandBuffer;
    cmd.reset();

    cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    // transition our main draw image into general layout so we can write into it
    // we will overwrite it all so we dont care about what was the older layout
    vk::tool::TransitionImage(cmd, DrawImage->image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

    draw_background(cmd);

    vk::tool::TransitionImage(cmd, DrawImage->image, vk::ImageLayout::eGeneral, vk::ImageLayout::eAttachmentOptimal);

    draw_geometry(cmd);

    //transition the draw image and the swapchain image into their correct transfer layouts
    vk::tool::TransitionImage(cmd, DrawImage->image, vk::ImageLayout::eAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);
    vk::tool::TransitionImage(cmd, SwapchainImages[swapchainImageIndex.value], vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    vk::tool::CopyImageToImage(cmd, DrawImage->image, SwapchainImages[swapchainImageIndex.value], DrawExtent, SwapchainInfo.imageExtent);

    // set swapchain image layout to Present so we can show it on the screen
    vk::tool::TransitionImage(cmd, SwapchainImages[swapchainImageIndex.value], vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);

    cmd.end();

    vk::CommandBufferSubmitInfo cmdInfo(cmd);
    vk::SemaphoreSubmitInfo waitInfo(*GetCurrentFrame().SwapchainSemaphore, 1, vk::PipelineStageFlagBits2KHR::eColorAttachmentOutput);
    vk::SemaphoreSubmitInfo signalInfo(*GetCurrentFrame().RenderSemaphore, 1, vk::PipelineStageFlagBits2KHR::eAllGraphics);

    vk::SubmitInfo2 submit({}, waitInfo, cmdInfo, signalInfo);
    GraphicsQueue.submit2(submit, *GetCurrentFrame().RenderFence);

    //present
    vk::PresentInfoKHR presentInfo(*GetCurrentFrame().RenderSemaphore, *Swapchain, swapchainImageIndex.value);
    auto res = GraphicsQueue.presentKHR(presentInfo); assert(res == vk::Result::eSuccess);

    res = Device->waitForFences(*GetCurrentFrame().RenderFence, true, 1000000000); assert(res == vk::Result::eSuccess);

    FrameNumber++;
}

void VkContext::init_vulkan()
{
    //Create Instance
    vk::ApplicationInfo appInfo{};
    appInfo.sType = vk::StructureType::eApplicationInfo;
    appInfo.pApplicationName = "VoxelWorld";
    appInfo.applicationVersion = vk::makeVersion(1, 0, 0);
    appInfo.pEngineName = "VoxelWorld";
    appInfo.engineVersion = vk::makeVersion(1, 0, 0);
    appInfo.apiVersion = vk::ApiVersion13;

    //Instance extensions and layers
    unsigned int cnt;
    SDL_Vulkan_GetInstanceExtensions(sdl_window, &cnt, nullptr);
    std::vector<const char*> instanceExtensionNames(cnt);
    SDL_Vulkan_GetInstanceExtensions(sdl_window, &cnt, instanceExtensionNames.data());
    std::vector<char const*> instanceLayerNames;
    if (ENABLE_VALIDATION_LAYERS)
    {
        instanceExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instanceLayerNames.push_back("VK_LAYER_KHRONOS_validation");
    }
    vk::tool::CheckLayers(instanceLayerNames, vk::enumerateInstanceLayerProperties());

    vk::InstanceCreateInfo instanceCreateInfo(vk::InstanceCreateFlags(), &appInfo, instanceLayerNames, instanceExtensionNames);
    Instance = vk::createInstanceUnique(instanceCreateInfo);
    DebugMessenger = vk::tool::CreateDebugMessenger(*Instance);
    
    //Create Surface
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(sdl_window, VkInstance(*Instance), &surface))
        throw std::runtime_error("Failed to create SDL Vulkan surface.");
    Surface = vk::UniqueSurfaceKHR(surface, vk::ObjectDestroy<vk::Instance, vk::DispatchLoaderStatic>(*Instance));

    //Create Physical device
    PhysicalDevice = vk::tool::ChoosePhysicalDevice(Instance->enumeratePhysicalDevices());

    //Create Graphics queue
    vk::tool::QueueFamilyIndices queueFamilyIndices = vk::tool::FindQueueFamilies(PhysicalDevice);
    vk::DeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = vk::StructureType::eDeviceQueueCreateInfo;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndices.Main;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    //Device Features2
    auto dynamicRendering = vk::tool::DynamicRenderingFeature;
    auto sync2 = vk::tool::Sync2Feature;
    auto bufferDeviceAddress = vk::tool::BufferDeviceAddressFeatures;
    dynamicRendering.pNext = &sync2;
    sync2.pNext = &bufferDeviceAddress;

    //Create Logic device
    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.pNext = &dynamicRendering;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    vk::PhysicalDeviceFeatures deviceFeatures = PhysicalDevice.getFeatures();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    //Device extensions
    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME };
    vk::tool::CheckExtensions(PhysicalDevice.enumerateDeviceExtensionProperties(), deviceExtensions);
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    Device = PhysicalDevice.createDeviceUnique(deviceCreateInfo);
    GraphicsQueue = Device->getQueue(queueFamilyIndices.Main, 0);

    //VMA Allocator
    VmaAllocator allocator;
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = PhysicalDevice;
    allocatorInfo.device = *Device;
    allocatorInfo.instance = *Instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &allocator);
    Allocator.Set(allocator);
}

void VkContext::init_swapchain()
{
    auto deviceCapabilities = PhysicalDevice.getSurfaceCapabilitiesKHR(Surface.get());
    auto presentModes = PhysicalDevice.getSurfacePresentModesKHR(Surface.get());
    auto surfaceFormats = PhysicalDevice.getSurfaceFormatsKHR(Surface.get());
    if (presentModes.empty() || surfaceFormats.empty())
    {
        throw std::runtime_error("Swapchains not supported.");
    }
    auto extent = vk::tool::ChooseSwapExtent(w_width, w_height, deviceCapabilities);
    auto surfaceFormat = vk::tool::ChooseSwapSurfaceFormat(surfaceFormats);
    auto presentMode = vk::tool::ChooseSwapPresentMode(presentModes);
    uint32_t imageCount = std::min(deviceCapabilities.minImageCount + 1, deviceCapabilities.maxImageCount);
    create_swapchain(extent, surfaceFormat, presentMode, deviceCapabilities.currentTransform, imageCount);

    //Create DrawImage
    AllocatedImage drawImage;
    drawImage.imageFormat = vk::Format::eR16G16B16A16Sfloat;
    drawImage.imageExtent = vk::Extent3D(w_width, w_height, 1);

    vk::ImageUsageFlags drawImageUsages{};
    drawImageUsages |= vk::ImageUsageFlagBits::eTransferSrc;
    drawImageUsages |= vk::ImageUsageFlagBits::eTransferDst;
    drawImageUsages |= vk::ImageUsageFlagBits::eStorage;
    drawImageUsages |= vk::ImageUsageFlagBits::eColorAttachment;

    VkImageCreateInfo rimg_info = vk::tool::ImageCreateInfo(drawImage.imageFormat, drawImageUsages, drawImage.imageExtent);

    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(*Allocator, &rimg_info, &rimg_allocinfo, (VkImage*) & drawImage.image, &drawImage.allocation, nullptr);

    drawImage.imageView = Device->createImageViewUnique(vk::tool::ImageViewCreateInfo(drawImage.imageFormat, drawImage.image, vk::ImageAspectFlagBits::eColor));
    DrawImage = Allocator.AddImage(std::move(drawImage));
    
}

void VkContext::create_swapchain(vk::Extent2D extent, vk::SurfaceFormatKHR surfaceFormat, vk::PresentModeKHR presentMode, vk::SurfaceTransformFlagBitsKHR transfrom, uint32_t imageCount)
{
    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
    createInfo.surface = *Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment|vk::ImageUsageFlagBits::eTransferSrc| vk::ImageUsageFlagBits::eTransferDst;

    //using single queue from Main queue family
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    createInfo.queueFamilyIndexCount = 0; // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional

    createInfo.preTransform = transfrom;
    createInfo.presentMode = presentMode;
    createInfo.clipped = vk::True;
    createInfo.oldSwapchain = nullptr;

    Swapchain = Device->createSwapchainKHRUnique(createInfo);
    SwapchainImages = Device->getSwapchainImagesKHR(*Swapchain);
    SwapchainInfo = createInfo;

    for (int i = 0; i < SwapchainImages.size(); i++)
    {
        vk::ImageViewCreateInfo imageCreateInfo({}, SwapchainImages[i]);
        imageCreateInfo
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(SwapchainInfo.imageFormat)
            .setComponents(vk::ComponentMapping())
            .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
        SwapchainImageViews.emplace_back(Device->createImageViewUnique(imageCreateInfo));
    }
}

void VkContext::draw_background(vk::CommandBuffer cmd)
{
    float flash = std::abs(std::sin(FrameNumber / 120.f));
    vk::ClearColorValue clearValue(0.0f, 0.0f, flash, 1.0f);
    vk::ImageSubresourceRange clearRange(vk::ImageAspectFlagBits::eColor, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS);

    cmd.clearColorImage(DrawImage->image, vk::ImageLayout::eGeneral, clearValue, clearRange);
}

void VkContext::draw_geometry(vk::CommandBuffer cmd)
{
    // Begin a render pass connected to our draw image
    
    vk::RenderingAttachmentInfo colorAttachment = vk::tool::AttachmentInfo(*DrawImage->imageView, nullptr, vk::ImageLayout::eGeneral);

    vk::RenderingInfo renderInfo{};
    renderInfo.renderArea.extent = DrawExtent;
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttachment;
    cmd.beginRendering(renderInfo);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *GraphicsPipeline);

    // Set dynamic viewport and scissor
    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(DrawExtent.width);
    viewport.height = static_cast<float>(DrawExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    cmd.setViewport(0, viewport);

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = DrawExtent;

    cmd.setScissor(0, scissor);


    std::array<Vertex, 4> rect_vertices;

    rect_vertices[0].position = { 0.5,-0.5, 0 };
    rect_vertices[1].position = { 0.5,0.5, 0 };
    rect_vertices[2].position = { -0.5,-0.5, 0 };
    rect_vertices[3].position = { -0.5,0.5, 0 };

    rect_vertices[0].color = { 0,0, 0,1 };
    rect_vertices[1].color = { 0.5,0.5,0.5 ,1 };
    rect_vertices[2].color = { 1,0, 0,1 };
    rect_vertices[3].color = { 0,1, 0,1 };

    std::array<uint32_t, 6> rect_indices;

    rect_indices[0] = 0;
    rect_indices[1] = 1;
    rect_indices[2] = 2;

    rect_indices[3] = 2;
    rect_indices[4] = 1;
    rect_indices[5] = 3;


    auto rectangle = UploadMesh(rect_indices, rect_vertices);

    GPUDrawPushConstants push_constants;
    push_constants.worldMatrix = glm::mat4{ 1.f };
    push_constants.vertexBuffer = rectangle.vertexBufferAddress;
    cmd.pushConstants(*GraphicsPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(GPUDrawPushConstants), &push_constants);
    cmd.bindIndexBuffer(rectangle.indexBuffer->buffer, 0, vk::IndexType::eUint32);
    cmd.drawIndexed(6, 1, 0, 0, 0);

    cmd.endRendering();
}

void VkContext::init_commands()
{
    vk::CommandPoolCreateInfo commandPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, vk::tool::FindQueueFamilies(PhysicalDevice).Main);

    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
        Frames[i].CommandPool = Device->createCommandPoolUnique(commandPoolInfo);

        vk::CommandBufferAllocateInfo cmdAllocInfo(*Frames[i].CommandPool, vk::CommandBufferLevel::ePrimary, 1);
        auto commandBuffers = Device->allocateCommandBuffersUnique(cmdAllocInfo);
        Frames[i].MainCommandBuffer = std::move(commandBuffers.front());
    }

    //immediate submits
    ImmCommandPool = Device->createCommandPoolUnique(commandPoolInfo);
    vk::CommandBufferAllocateInfo cmdAllocInfo(*ImmCommandPool, vk::CommandBufferLevel::ePrimary, 1);
    auto commandBuffers = Device->allocateCommandBuffersUnique(cmdAllocInfo);
    ImmCommandBuffer = std::move(commandBuffers.front());
}

void VkContext::init_sync_structures()
{
    vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
    vk::SemaphoreCreateInfo semaphoreCreateInfo;

    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
        Frames[i].RenderFence = Device->createFenceUnique(fenceCreateInfo);
        Frames[i].SwapchainSemaphore = Device->createSemaphoreUnique(semaphoreCreateInfo);
        Frames[i].RenderSemaphore = Device->createSemaphoreUnique(semaphoreCreateInfo);
    }

    ImmFence = Device->createFenceUnique(fenceCreateInfo);
}

void VkContext::init_graphics_pipeline()
{
    // Load shaders
    std::vector<char> vertShaderCode = vk::tool::ReadFileBinary("res/Shaders/vert.spv");
    std::vector<char> fragShaderCode = vk::tool::ReadFileBinary("res/Shaders/frag.spv");

    auto vertShaderModule = Device->createShaderModuleUnique(vk::tool::ShaderModuleCreateInfo(vertShaderCode));
    auto fragShaderModule = Device->createShaderModuleUnique(vk::tool::ShaderModuleCreateInfo(fragShaderCode));

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        vk::tool::ShaderStageCreateInfo(*vertShaderModule, vk::ShaderStageFlagBits::eVertex, "main"),
        vk::tool::ShaderStageCreateInfo(*fragShaderModule, vk::ShaderStageFlagBits::eFragment, "main")
    };

    // Dynamic states
    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.setDynamicStates(dynamicStates);

    // Vertex input state
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    // Input assembly state
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

    // Viewport state
    vk::PipelineViewportStateCreateInfo viewportState{};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Rasterizer state
    vk::PipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisample state
    vk::PipelineMultisampleStateCreateInfo multisampling{};
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    // Color blend state
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
    colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
    colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
    colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    
    // Push constants
    vk::PushConstantRange pushConstants{};
    pushConstants.offset = 0;
    pushConstants.size = sizeof(GPUDrawPushConstants);
    pushConstants.stageFlags = vk::ShaderStageFlagBits::eVertex;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstants;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    GraphicsPipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);

    // Dynamic rendering pipeline
    vk::Format colorFormat = vk::Format::eR16G16B16A16Sfloat;
    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;

    vk::GraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = *GraphicsPipelineLayout;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pNext = &pipelineRenderingCreateInfo;

    auto pipelines = Device->createGraphicsPipelinesUnique(nullptr, pipelineInfo);
    GraphicsPipeline = std::move(pipelines.value.front());
}
GPUMeshBuffers VkContext::UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices)
{
    const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

    GPUMeshBuffers newSurface;

    //create vertex buffer
    newSurface.vertexBuffer = Allocator.CreateBufferUnique(vertexBufferSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        VMA_MEMORY_USAGE_GPU_ONLY);

    //find the adress of the vertex buffer
    vk::BufferDeviceAddressInfo deviceAdressInfo(newSurface.vertexBuffer->buffer);
    newSurface.vertexBufferAddress = Device->getBufferAddress(deviceAdressInfo);

    //create index buffer
    newSurface.indexBuffer = Allocator.CreateBufferUnique(indexBufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        VMA_MEMORY_USAGE_GPU_ONLY);

    AllocatedBuffer staging = Allocator.CreateBuffer(vertexBufferSize + indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);
    void* data = staging.allocation->GetMappedData();

    // copy vertex buffer
    memcpy(data, vertices.data(), vertexBufferSize);
    // copy index buffer
    memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

    auto a = newSurface.vertexBuffer;

    ImmediateSubmit([&](vk::CommandBuffer cmd) {
        vk::BufferCopy vertexCopy{ 0 };
        vertexCopy.dstOffset = 0;
        vertexCopy.srcOffset = 0;
        vertexCopy.size = vertexBufferSize;

        cmd.copyBuffer(staging.buffer, newSurface.vertexBuffer->buffer, vertexCopy);

        vk::BufferCopy indexCopy{ 0 };
        indexCopy.dstOffset = 0;
        indexCopy.srcOffset = vertexBufferSize;
        indexCopy.size = indexBufferSize;

        cmd.copyBuffer(staging.buffer, newSurface.indexBuffer->buffer, indexCopy);
    });

    vmaDestroyBuffer(*Allocator, staging.buffer, staging.allocation);

    return newSurface;
}

void VkContext::ImmediateSubmit(std::function<void(vk::CommandBuffer cmd)>&& function)
{
    vk::Device device = *Device;
    vk::Queue graphicsQueue = GraphicsQueue;
    vk::Fence immFence = *ImmFence;
    vk::CommandBuffer immCommandBuffer = *ImmCommandBuffer;

    device.resetFences(immFence);
    immCommandBuffer.reset({});

    immCommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    function(immCommandBuffer);

    immCommandBuffer.end();

    vk::CommandBufferSubmitInfo cmdInfo = vk::CommandBufferSubmitInfo(immCommandBuffer);
    vk::SubmitInfo2 submit({}, nullptr, cmdInfo, nullptr);

    // Submit command buffer to the queue and execute it.
    // _renderFence will now block until the graphic commands finish execution
    graphicsQueue.submit2(submit, immFence);

    device.waitForFences(immFence, VK_TRUE, 9999999999);

}
