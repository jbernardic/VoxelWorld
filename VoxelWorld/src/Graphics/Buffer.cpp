#include "Buffer.h"
#include "../Debug/ErrorHandler.h"
#include <glm/glm.hpp>

Buffer::Buffer(GLuint id, GLuint type, size_t size) : id(id), type(type), size(size) {}

Buffer::~Buffer()
{
	lcall(glDeleteBuffers(1, &id));
}

void Buffer::Bind() const
{
	lcall(glBindBuffer(type, id));
}

void Buffer::UnBind() const
{
	lcall(glBindBuffer(type, 0));
}

void Buffer::SubData(size_t size, void* data, unsigned int offset) const
{
	Bind();
	lcall(glBufferSubData(type, offset, size, data));
}

void Buffer::BindBase(int bindingPoint) const
{
	lcall(glBindBufferBase(type, bindingPoint, id));
}

std::shared_ptr<Buffer> Buffer::Create(Type type, size_t size, void* data)
{
	GLuint glType = GetBufferType(type);

	GLuint id;
	lcall(glGenBuffers(1, &id));
	lcall(glBindBuffer(glType, id));
	lcall(glBufferData(glType, size, data, GL_STATIC_DRAW));
	return std::make_shared<Buffer>(id, glType, size);
}
