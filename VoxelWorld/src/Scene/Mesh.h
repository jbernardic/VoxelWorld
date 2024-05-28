#pragma once
#include "../Vulkan/VkTypes.h"

struct MeshSurfaceInfo
{
    uint32_t startIndex;
    uint32_t count;
};

class Mesh
{
public:
    Mesh(const GPUMeshBuffers& meshBuffers, const std::string& name, const std::vector<MeshSurfaceInfo>& surfaces);
    const std::string Name;
    const std::vector<MeshSurfaceInfo> Surfaces;
    GPUMeshBuffers Buffers;
};