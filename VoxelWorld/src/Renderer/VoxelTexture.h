#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include <memory>

class VoxelTexture
{
	//todo: abstract opengl away
public:
	VoxelTexture(GLuint textureID, glm::ivec3 size, glm::vec4* data);
	~VoxelTexture();
	static std::shared_ptr<VoxelTexture> Create(glm::ivec3 size, glm::vec4* data);
	const glm::ivec3 Size;
	const void Bind() const;
	const void UnBind() const;
private:
	std::vector<glm::vec4> data;
	const GLuint textureID;
};