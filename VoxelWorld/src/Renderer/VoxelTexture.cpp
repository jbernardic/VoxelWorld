#include "VoxelTexture.h"

VoxelTexture::VoxelTexture(GLuint textureID, glm::ivec3 size, glm::vec4* data)
	: textureID(textureID), Size(size)
{
	this->data.assign(data, data + size.x*size.y*size.z);
}

VoxelTexture::~VoxelTexture()
{
	glDeleteTextures(1, &textureID);
}

std::shared_ptr<VoxelTexture> VoxelTexture::Create(glm::ivec3 size, glm::vec4* data)
{

	GLuint _texture;
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_3D, _texture);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, size.x, size.y, size.z, 0, GL_RGBA, GL_FLOAT, data);
	
	return std::make_shared<VoxelTexture>(_texture, size, data);
}

const void VoxelTexture::Bind() const
{
	glBindTextureUnit(0, textureID);
}

const void VoxelTexture::UnBind() const
{
	glBindTextureUnit(0, textureID);
}
