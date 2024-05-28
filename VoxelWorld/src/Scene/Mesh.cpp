#include "Mesh.h"

Mesh::Mesh(const GPUMeshBuffers& meshBuffers, const std::string& name, const std::vector<MeshSurfaceInfo>& surfaces)
	: Buffers(meshBuffers), Name(name), Surfaces(surfaces)
{
}