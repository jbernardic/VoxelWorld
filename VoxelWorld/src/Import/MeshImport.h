#pragma once
#include "../Core/Application.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include "../Scene/Mesh.h"

namespace Import {
	std::vector<std::shared_ptr<Mesh>> LoadMeshes(std::filesystem::path filePath);
}