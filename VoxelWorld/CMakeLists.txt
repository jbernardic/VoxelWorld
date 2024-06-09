# CMakeList.txt : CMake project for VoxelWorld, include source and define
# project specific logic here.
#
set(SDL_PATH "${PROJECT_SOURCE_DIR}/vendor/SDL2-2.26.2")
set(GLM_PATH "${PROJECT_SOURCE_DIR}/vendor/glm")
set(STB_IMAGE_PATH "${PROJECT_SOURCE_DIR}/vendor/stb_image")

list(APPEND CMAKE_PREFIX_PATH "${SDL_PATH}")

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable (VoxelWorld "src/Main.cpp"   "src/Game.cpp" "src/Game.h"   "src/Core/Input.cpp" "src/Core/Input.h" "src/Core/ResourceManager.h" "src/Core/ResourceManager.cpp" "src/Core/Application.cpp" "src/Core/Application.h" "src/Scene/Camera.cpp" "src/Scene/Camera.h" "src/ogt_vox.h" "src/Vulkan/VkTypes.h" "src/Vulkan/VkContext.h" "src/Vulkan/VkContext.cpp" "src/Vulkan/VkTools.h" "src/Vulkan/VkImages.h" "src/Vulkan/VkDebug.h" "src/Vulkan/VkAllocator.h" "src/Asset/ModelAsset.h" "src/Asset/ModelAsset.cpp"      "src/Scene/Scene.h" "src/Scene/Scene.cpp"   "src/Helpers/Math.h"     "src/Scene/Common.h" "src/Animation/AnimationController.h" "src/Animation/Common.h" "src/Animation/AnimationController.cpp")

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} "${GLM_PATH}" "${STB_IMAGE_PATH}")
target_link_libraries(VoxelWorld fastgltf::fastgltf ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} "GPUOpen::VulkanMemoryAllocator" "Vulkan::Vulkan")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VoxelWorld PROPERTY CXX_STANDARD 20)
endif()

add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/res
        COMMENT "Copying resources into binary directory")

add_dependencies(VoxelWorld copy_resources)
