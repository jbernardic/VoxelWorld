# CMakeList.txt : CMake project for VoxelWorld, include source and define
# project specific logic here.
#
set(SDL_PATH "${PROJECT_SOURCE_DIR}/vendor/SDL2-2.26.2")
set(SDL_IMAGE_PATH "${PROJECT_SOURCE_DIR}/vendor/SDL2_image-2.6.2")
set(ASSIMP_PATH "${PROJECT_SOURCE_DIR}/vendor/assimp")
set(GLM_PATH "${PROJECT_SOURCE_DIR}/vendor/glm")

list(APPEND CMAKE_PREFIX_PATH "${SDL_PATH}")
list(APPEND CMAKE_PREFIX_PATH "${SDL_IMAGE_PATH}")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(Vulkan REQUIRED)

add_executable (VoxelWorld "src/Main.cpp"   "src/Game.cpp" "src/Game.h"   "src/Core/Input.cpp" "src/Core/Input.h" "src/Core/ResourceManager.h" "src/Core/ResourceManager.cpp" "src/Core/Application.cpp" "src/Core/Application.h" "src/Renderer/Camera.cpp" "src/Renderer/Camera.h" "src/ogt_vox.h" "src/Vulkan/VkTypes.h" "src/Vulkan/VkContext.h" "src/Vulkan/VkContext.cpp" "src/Vulkan/VkUtils.h" "src/Vulkan/VkImages.h" "src/Vulkan/VkDebug.h")

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} "${GLM_PATH}" "${ASSIMP_PATH}/include")
target_link_libraries(VoxelWorld ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} "${ASSIMP_PATH}/bin/assimp-vc143-mt.lib" "GPUOpen::VulkanMemoryAllocator" "Vulkan::Vulkan")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VoxelWorld PROPERTY CXX_STANDARD 20)
endif()

add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/res
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/res
        COMMENT "Copying resources into binary directory")

add_dependencies(VoxelWorld copy_resources)
