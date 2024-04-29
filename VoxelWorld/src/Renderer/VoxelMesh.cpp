#include "VoxelMesh.h"


VoxelMesh::VoxelMesh(std::array<glm::vec3, 8> vertices, std::array<int, 36> indices, glm::ivec3 size) 
	: Vertices(vertices), Indices(indices), Size(size)
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

    auto mesh = std::make_shared<VoxelMesh>(vertices, indices, size);

    mesh->VA = VertexArray::Create();
    mesh->VA->Bind();
    mesh->VB = Buffer::Create(Buffer::Type::VertexBuffer, mesh->Vertices.size() * sizeof(glm::vec3), (void*)mesh->Vertices.data());
    mesh->IB = Buffer::Create(Buffer::Type::IndexBuffer, mesh->Indices.size() * sizeof(unsigned int), (void*)mesh->Indices.data());
    mesh->VA->AddBuffer(mesh->VB.get(), 0, 3, GL_FLOAT, sizeof(glm::vec3), 0);
    mesh->VA->UnBind();
    mesh->DataTexture = Texture<uint8_t>::Create3D_U8(size.x, size.y, size.z, data);
    mesh->PaletteTexture = Texture<glm::vec4>::Create1D_32F(256, palette);

    return mesh;
}

void VoxelMesh::UpdateData(glm::ivec3 offset, glm::ivec3 size, const uint8_t* data) const
{
    DataTexture->Update3D_U8(offset, size, Size, data);
}

uint8_t VoxelMesh::GetVoxel(glm::ivec3 position) const
{
    return DataTexture->GetPixel3D(position, Size);
}
