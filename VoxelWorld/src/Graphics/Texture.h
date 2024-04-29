#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include <memory>

template <class T>
class Texture
{
public:

	Texture(GLuint textureID, size_t size, const T* data) : id(textureID), data(data, data+size) {}

	~Texture()
	{
		glDeleteTextures(1, &id);
	}

	void Bind(int binding = 0) const
	{
		glBindTextureUnit(binding, id);
	}

	void Update3D_U8(glm::ivec3 offset, glm::ivec3 size, glm::ivec3 image_size, const uint8_t* data)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTextureSubImage3D(id, 0, offset.x, offset.y, offset.z, size.x, size.y, size.z, GL_RED, GL_UNSIGNED_BYTE, data);
		for (int x = 0; x < size.x; x++)
		{
			for (int y = 0; y < size.y; y++)
			{
				for (int z = 0; z < size.z; z++)
				{
					int x1 = offset.x + x;
					int y1 = offset.y + y;
					int z1 = offset.z + z;
					this->data[x1 + image_size.x * (y1 + image_size.y * z1)] = data[x + size.x * (y + size.y * z)];
				}
			}
		}
	}

	T GetPixel3D(glm::ivec3 coord, glm::ivec3 image_size)
	{
		return data[coord.x + image_size.x * (coord.y + image_size.y * coord.z)];
	}

	T GetPixel1D(int index, size_t image_size)
	{
		return data[index];
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
	const GLuint id;
};