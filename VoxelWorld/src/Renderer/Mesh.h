#pragma once
#include "../Vulkan/VkTypes.h"

struct GeoSurface
{
    uint32_t startIndex;
    uint32_t count;
};

class Mesh
{
public:
    Mesh(const GPUMeshBuffers& meshBuffers, const std::string& name, const std::vector<GeoSurface>& surfaces);
    const std::string Name;
    const std::vector<GeoSurface> Surfaces;
    GPUMeshBuffers Buffers;
    
};