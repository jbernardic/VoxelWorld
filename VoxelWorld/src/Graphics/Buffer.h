#pragma once
#include <glad/glad.h>
#include <iostream>
class Buffer //VBO, IBO
{
public:
	
	enum class BufferType
	{
		VertexBuffer, IndexBuffer, UniformBuffer
	};

	Buffer(GLuint id, BufferType type);
	~Buffer();

	const virtual void Bind() const;
	const virtual void UnBind() const;
	const virtual void SubData(void* data, size_t size, unsigned int offset = 0) const;
	const GLuint& ID = m_ID;

	static std::shared_ptr<Buffer> CreateVertexBuffer(void* data, size_t size);
	static std::shared_ptr<Buffer> CreateVertexBuffer(size_t size);
	static std::shared_ptr<Buffer> CreateIndexBuffer(void* data, size_t size);
	static std::shared_ptr<Buffer> CreateIndexBuffer(size_t size);

protected:
	GLuint m_ID;
	BufferType m_Type;
};