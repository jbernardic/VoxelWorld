#pragma once
#include <iostream>
#include<string>
#include"../Debug/ErrorHandler.h"
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <vector>

struct ShaderSrc
{
	std::string source;
	GLuint type;
	unsigned int line_count = 0;
};

class Shader
{
public:
	Shader(const std::string& path);
	~Shader();
	std::vector<ShaderSrc> GetShaderSource(const std::string& path);
	void SetVec4(const char* name, glm::vec4 vec) const;
	void SetVec2(const char* name, glm::vec2 vec) const;
	void SetVec3(const char* name, glm::vec3 vec);
	void SetFloat(const char* name, float a) const;
	void SetInt(const char* name, int a) const;
	void SetMat4(const char* name, const glm::mat4 value) const;
	void SetIntArray(const char* name, int* arr, int length);
	void Bind() const;
	void UnBind() const;
	GLuint CompileShader(GLuint type, const std::string& source);
	GLuint CreateShader(const std::vector<ShaderSrc>& srcs);
	const GLuint& ID = m_ID;
private:
	GLuint m_ID = 0;
};

