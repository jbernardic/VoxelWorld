#include "VertexArray.h"
#include "../Debug/ErrorHandler.h"
#include <iostream>
#include <glad/glad.h>
#include "Buffer.h"

VertexArray::VertexArray(GLuint id) : m_ID(id) { }

VertexArray::~VertexArray()
{
	lcall(glDeleteVertexArrays(1, &m_ID));
}

void VertexArray::Bind() const
{
	lcall(glBindVertexArray(m_ID));
}

void VertexArray::UnBind() const
{
	lcall(glBindVertexArray(0));
}

std::shared_ptr<VertexArray> VertexArray::Create()
{
	GLuint id;
	lcall(glGenVertexArrays(1, &id));
	std::shared_ptr<VertexArray> va = std::make_shared<VertexArray>(id);
	return va;
}

void VertexArray::AddBuffer(Buffer* bf, GLuint index, GLint count, GLenum type, GLsizei stride, GLuint offset)
{
	Bind();
	bf->Bind();

	if (m_VB != bf) m_Offset = 0;
	if (offset != 0) m_Offset = offset;

	if (type == GL_FLOAT)
	{
		glVertexAttribPointer(index, count, GL_FLOAT, GL_FALSE, stride, (void*)(m_Offset));
	}

	if (offset == 0) m_Offset += count*sizeof(float);
	glEnableVertexAttribArray(index);
	m_VB = bf;
}