#include "VoxelMesh.h"


VoxelMesh::VoxelMesh(std::array<glm::vec3, 8> vertices, std::array<int, 36> indices) 
	: Vertices(vertices), Indices(indices)
{

    VA = VertexArray::Create();
    VA->Bind();
    VB = Buffer::CreateVertexBuffer((void*)Vertices.data(), Vertices.size() * sizeof(glm::vec3));
    IB = Buffer::CreateIndexBuffer((void*)Indices.data(), Indices.size() * sizeof(unsigned int));
    VA->AddBuffer(VB.get(), 0, 3, GL_FLOAT, sizeof(glm::vec3), 0);
    VA->UnBind();
}


std::shared_ptr<VoxelMesh> VoxelMesh::Create(glm::vec3 size, glm::vec3 position)
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

    return std::make_shared<VoxelMesh>(vertices, indices);
}