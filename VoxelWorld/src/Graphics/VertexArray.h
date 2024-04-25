#pragma once
#include "Buffer.h"
class VertexArray
{

public:
	VertexArray(GLuint id);
	~VertexArray();
	void AddBuffer(Buffer* bf, GLuint index, GLint count, GLenum type, GLsizei stride, GLuint offset=0);
	void Bind() const;
	void UnBind() const;

	static std::shared_ptr<VertexArray> Create();

	const int& ID = m_ID;
private:
	unsigned int m_ID;
	unsigned int m_Offset = 0;
	Buffer* m_VB = nullptr;
};