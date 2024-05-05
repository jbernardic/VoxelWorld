#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include <memory>
#include "../Debug/ErrorHandler.h"

template <class T>
class Texture
{
public:

	Texture(GLuint textureID, size_t size, const T* data) : id(textureID) {}

	~Texture()
	{
		glDeleteTextures(1, &id);
	}

	void Bind(int binding = 0) const
	{
		glBindTextureUnit(binding, id);
	}

	void SubImage3D_RED_U8(glm::ivec3 offset, glm::ivec3 size, glm::ivec3 image_size, const uint8_t* data, GLint level=0) const
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTextureSubImage3D(id, level, offset.x, offset.y, offset.z, size.x, size.y, size.z, GL_RED, GL_UNSIGNED_BYTE, data);
	}

	void SubImage3D_RED_32F(glm::ivec3 offset, glm::ivec3 size, glm::ivec3 image_size, const uint8_t* data, GLint level = 0) const
	{
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTextureSubImage3D(id, level, offset.x, offset.y, offset.z, size.x, size.y, size.z, GL_RED, GL_FLOAT, data);
	}

	void SubImage3D_RGBA_32F(glm::ivec3 offset, glm::ivec3 size, glm::ivec3 image_size, const uint8_t* data, GLint level = 0) const
	{
		lcall(glTextureSubImage3D(id, level, offset.x, offset.y, offset.z, size.x, size.y, size.z, GL_RGBA, GL_FLOAT, data));
	}

	void GenerateMipmap() const
	{
		glGenerateTextureMipmap(id);
	}

	static std::shared_ptr<Texture<T>> Create1D_RGBA_32F(size_t size, const T* data)
	{
		GLuint _texture;
		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_1D, _texture);

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, size, 0, GL_RGBA, GL_FLOAT, data);

		return std::make_shared<Texture<T>>(_texture, size, data);
	}
	static std::shared_ptr<Texture<T>> Create3D_RED_U8(glm::ivec3 size, const T* data)
	{
		GLuint _texture;
		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_3D, _texture);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, size.x, size.y, size.z, 0, GL_RED, GL_UNSIGNED_BYTE, data);
		return std::make_shared<Texture<T>>(_texture, size.x * size.y * size.z, data);
	}

	static std::shared_ptr<Texture<T>> CreateOpacityMap(glm::ivec3 size, const T* data)
	{
		GLuint _texture;
		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_3D, _texture);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, size.x, size.y, size.z, 0, GL_RED, GL_UNSIGNED_BYTE, data);
		return std::make_shared<Texture<T>>(_texture, size.x * size.y * size.z, data);
	}

private:
	const GLuint id;
};