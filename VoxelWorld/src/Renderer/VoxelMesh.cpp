#include "VoxelMesh.h"


VoxelMesh::VoxelMesh(std::array<glm::vec3, 8> vertices, std::array<int, 36> indices, glm::ivec3 size, const uint8_t* data)
	: Vertices(vertices), Indices(indices), Size(size), data(data, data+size.x*size.y*size.z)
{}


std::shared_ptr<VoxelMesh> VoxelMesh::Create(glm::ivec3 size, const uint8_t* data, const glm::vec4* palette = nullptr)
{

    std::array<glm::vec3, 8> vertices = {
        // Front face
        glm::vec3(0, 0, size.z),  // Bottom-left
        glm::vec3(size.x, 0, size.z),  // Bottom-right
        glm::vec3(size.x, size.y,  size.z),  // Top-right
        glm::vec3(0,  size.y,  size.z),  // Top-left

        // Back face
        glm::vec3(0, 0, 0),  // Bottom-left
        glm::vec3(size.x, 0, 0),  // Bottom-right
        glm::vec3(size.x,  size.y, 0),  // Top-right
        glm::vec3(0, size.y, 0)   // Top-left
    };

    std::array<int, 36> indices = {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Right face
        1, 5, 6,
        6, 2, 1,

        // Back face
        7, 6, 5,
        5, 4, 7,

        // Left face
        4, 0, 3,
        3, 7, 4,

        // Bottom face
        4, 5, 1,
        1, 0, 4,

        // Top face
        3, 2, 6,
        6, 7, 3
    };

    auto mesh = std::make_shared<VoxelMesh>(vertices, indices, size, data);

    mesh->VA = VertexArray::Create();
    mesh->VA->Bind();
    mesh->VB = Buffer::Create(Buffer::Type::VertexBuffer, mesh->Vertices.size() * sizeof(glm::vec3), (void*)mesh->Vertices.data());
    mesh->IB = Buffer::Create(Buffer::Type::IndexBuffer, mesh->Indices.size() * sizeof(unsigned int), (void*)mesh->Indices.data());
    mesh->VA->AddBuffer(mesh->VB.get(), 0, 3, GL_FLOAT, sizeof(glm::vec3), 0);
    mesh->VA->UnBind();
    mesh->VoxelTexture = Texture<uint8_t>::Create3D_RED_U8(size, data);
    mesh->PaletteTexture = Texture<glm::vec4>::Create1D_RGBA_32F(256, palette);
    mesh->NormalMapBuffer = Buffer::Create(Buffer::Type::ShaderStorageBuffer, size.x*size.y*size.z * (sizeof(glm::vec3) + sizeof(float)), nullptr);

    std::vector<uint8_t> opacityData(size.x*size.y*size.z);
    for (int i = 0; i < size.x * size.y * size.z; i++) opacityData[i] = data[i] > 0.0 ? 255 : 0;
    mesh->OpacityMap = Texture<uint8_t>::CreateOpacityMap(size, opacityData.data());
    mesh->OpacityMap->GenerateMipmap();

    return mesh;
}

void VoxelMesh::UpdateVoxels(glm::ivec3 offset, glm::ivec3 size, const uint8_t* data)
{
    VoxelTexture->SubImage3D_RED_U8(offset, size, Size, data);
    std::vector<uint8_t> opacityData(size.x * size.y * size.z);
    for (int i = 0; i < size.x * size.y * size.z; i++) opacityData[i] = data[i] > 0.0 ? 255 : 0;
    OpacityMap->SubImage3D_RED_U8(offset, size, Size, opacityData.data());
    OpacityMap->GenerateMipmap();
    for (int x = 0; x < size.x; x++)
    {
    	for (int y = 0; y < size.y; y++)
    	{
    		for (int z = 0; z < size.z; z++)
    		{
    			int x1 = offset.x + x;
    			int y1 = offset.y + y;
    			int z1 = offset.z + z;
    			this->data[x1 + Size.x * (y1 + Size.y * z1)] = data[x + size.x * (y + size.y * z)];
    		}
    	}
    }
}

uint8_t VoxelMesh::GetVoxel(glm::ivec3 position) const
{
    return data[position.x + Size.x * (position.y + Size.y * position.z)];
}
