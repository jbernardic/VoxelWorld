#pragma once
#include "../Core/Application.h"
#include "../Renderer/Mesh.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

namespace GLTF {
	std::vector<std::shared_ptr<Mesh>> LoadMeshes(std::filesystem::path filePath);
}