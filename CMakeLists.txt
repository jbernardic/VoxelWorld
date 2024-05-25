# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
unset(CMAKE_PREFIX_PATH CACHE)

project ("VoxelWorld")

add_subdirectory ("vendor/VulkanMemoryAllocator")
add_subdirectory ("VoxelWorld")

