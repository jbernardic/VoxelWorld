#include "Mesh.h"

Mesh::Mesh(const GPUMeshBuffers& meshBuffers, const std::string& name, const std::vector<GeoSurface>& surfaces)
	: Buffers(meshBuffers), Name(name), Surfaces(surfaces)
{}
