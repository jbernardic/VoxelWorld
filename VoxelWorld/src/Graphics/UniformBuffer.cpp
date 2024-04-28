#include "UniformBuffer.h"
#include "../Debug/ErrorHandler.h"
#include <glm/glm.hpp>

UniformBuffer::UniformBuffer(GLuint id, GLuint bindingPoint)
	: bindingPoint(bindingPoint), id(id)
{}

std::shared_ptr<UniformBuffer> UniformBuffer::Create(size_t size, GLuint bindingPoint)
{
	GLuint id;
	glGenBuffers(1, &id);

	glBindBuffer(GL_UNIFORM_BUFFER, id);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, id, 0, size);
	return std::make_shared<UniformBuffer>(id, bindingPoint);
}

const void UniformBuffer::Bind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, id);
}

const void UniformBuffer::UnBind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

const void UniformBuffer::SubData(void* data, size_t size, unsigned int offset) const
{
	Bind();
	lcall(glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data));
}
