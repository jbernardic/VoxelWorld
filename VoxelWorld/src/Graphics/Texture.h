#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include <memory>

template <class T>
class Texture
{
public:

	Texture(GLuint textureID, size_t size, const T* data) : textureID(textureID), data(data, data+size) {}

	~Texture()
	{
		glDeleteTextures(1, &textureID);
	}

	const void Bind(int binding = 0) const
	{
		glBindTextureUnit(binding, textureID);
	}

	static std::shared_ptr<Texture<T>> Create1D_32F(size_t size, const T* data)
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
	static std::shared_ptr<Texture<T>> Create3D_U8(size_t sizeX, size_t sizeY, size_t sizeZ, const T* data)
	{
		GLuint _texture;
		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_3D, _texture);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, sizeX, sizeY, sizeZ, 0, GL_RED, GL_UNSIGNED_BYTE, data);

		return std::make_shared<Texture<T>>(_texture, sizeX*sizeY*sizeZ, data);
	}
private:
	std::vector<T> data;
	std::vector<glm::vec4> palette;
	const GLuint textureID;
};