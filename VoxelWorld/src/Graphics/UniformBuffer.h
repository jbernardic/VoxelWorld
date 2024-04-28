#pragma once
#include "Buffer.h"

class UniformBuffer
{
public:
	UniformBuffer(GLuint id, GLuint bindingPoint);
	static std::shared_ptr<UniformBuffer> Create(size_t size, GLuint bindingPoint);
	const void Bind() const;
	const void UnBind() const;
	const void SubData(void* data, size_t size, unsigned int offset) const;
private:
	GLuint bindingPoint;
	GLuint id;
};