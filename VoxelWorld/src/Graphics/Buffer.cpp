#include "Buffer.h"
#include "../Debug/ErrorHandler.h"
#include <glm/glm.hpp>

Buffer::Buffer(GLuint id, BufferType type) : m_ID(id), m_Type(type) {}

Buffer::~Buffer()
{
	lcall(glDeleteBuffers(1, &m_ID));
}

const void Buffer::Bind() const
{
	switch (m_Type)
	{
	case Buffer::BufferType::VertexBuffer:
		lcall(glBindBuffer(GL_ARRAY_BUFFER, m_ID));
		break;
	case Buffer::BufferType::IndexBuffer:
		lcall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID));
		break;
	default:
		break;
	}
}

const void Buffer::UnBind() const
{
	switch (m_Type)
	{
	case Buffer::BufferType::VertexBuffer:
		lcall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		break;
	case Buffer::BufferType::IndexBuffer:
		lcall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		break;
	default:
		break;
	}
}

const void Buffer::SubData(size_t size, void* data, unsigned int offset) const
{
	Bind();
	lcall(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
}

std::shared_ptr<Buffer> Buffer::CreateVertexBuffer(size_t size, void* data)
{
	GLuint id;
	lcall(glGenBuffers(1, &id));
	lcall(glBindBuffer(GL_ARRAY_BUFFER, id));
	lcall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	return std::make_shared<Buffer>(id, BufferType::VertexBuffer);
}

std::shared_ptr<Buffer> Buffer::CreateIndexBuffer(size_t size, void* data)
{
	GLuint id;
	lcall(glGenBuffers(1, &id));
	lcall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id));
	lcall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	return std::make_shared<Buffer>(id, BufferType::IndexBuffer);
}
