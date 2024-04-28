#pragma once
#include <glad/glad.h>
#include <iostream>
class Buffer //VBO, IBO
{
public:
	
	enum class Type
	{
		VertexBuffer, IndexBuffer, UniformBuffer, ShaderStorageBuffer
	};

	static GLuint GetBufferType(Type type)
	{
		switch (type)
		{
		case Type::VertexBuffer:
			return GL_ARRAY_BUFFER;
		case Type::IndexBuffer:
			return GL_ELEMENT_ARRAY_BUFFER;
		case Type::UniformBuffer:
			return GL_UNIFORM_BUFFER;
		case Type::ShaderStorageBuffer:
			return GL_SHADER_STORAGE_BUFFER;
		}
	}

	Buffer(GLuint id, GLuint type, size_t size=0);
	~Buffer();

	const void Bind() const;
	const void UnBind() const;
	const void SubData(size_t size, void* data, unsigned int offset = 0) const;
	const void BindBase(int bindingPoint) const;

	const GLuint& ID = id;

	static std::shared_ptr<Buffer> Create(Type type, size_t size=0, void* data = nullptr);

private:
	GLuint id;
	GLuint type;
	size_t size;
};