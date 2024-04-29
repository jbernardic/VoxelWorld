#include "Shader.h"
#include "../Debug/ErrorHandler.h"
#include <fstream>
#include <string>
#include <vector>

using namespace std;

Shader::Shader(const std::string& path)
{
	vector<ShaderSrc> srcs = GetShaderSource(path);
	m_ID = CreateShader(srcs);
}

Shader::~Shader()
{
	std::cout << "shader dtor" << std::endl;
	glDeleteProgram(m_ID);
}

std::vector<ShaderSrc> Shader::GetShaderSource(const string& path)
{
	ifstream stream(path);
	assert(stream.is_open());
	string line;
	GLuint type = 0;
	vector<ShaderSrc> shaders;
	vector<string> lines;
	int line_cnt = 0;
	while (getline(stream, line))
	{
		line_cnt++;
		std::size_t include = line.find("#include ");
		std::size_t version = line.find("#version ");
		if (version != std::string::npos)
		{
			lines.push_back(line);
			lines.push_back("#line " + to_string(line_cnt+1));
		}
		else if (include != std::string::npos)
		{
			std::string res = line.substr(include + 9);
			ifstream f("res/Shaders/" + res);
			assert(f.is_open());
			string line2;
			while (getline(f, line2))
			{
				lines.push_back(line2);
			}
			lines.push_back("#line " + to_string(line_cnt+1));
		}
		else lines.push_back(line);
	}
	for (string& line : lines)
	{
		if (line == "#shader vertex")
		{
			type = GL_VERTEX_SHADER;
			shaders.emplace_back("", type);
		}
		else if (line == "#shader fragment")
		{
			type = GL_FRAGMENT_SHADER;
			shaders.emplace_back("", type);
		}
		else if (line == "#shader compute")
		{
			type = GL_COMPUTE_SHADER;
			shaders.emplace_back("", type);
		}
		else
		{
			assert(type != 0);
			shaders[shaders.size() - 1].source += line + "\n";
		}
	}
	return shaders;
}



void Shader::Bind() const
{
	lcall(glUseProgram(m_ID));
}

void Shader::UnBind() const
{
	lcall(glUseProgram(0));
}

GLuint Shader::CompileShader(GLuint type, const string& source)
{

	GLuint id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (!result)
	{

		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = new char[length];
		glGetShaderInfoLog(id, length, &length, message);
		cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << endl;
		cout << message << endl;
	}
	return id;
}

GLuint Shader::CreateShader(const vector<ShaderSrc>& srcs)
{

	GLuint program = glCreateProgram();

	vector<GLuint> shader_objs;
	for (const ShaderSrc& src : srcs)
	{
		GLuint shader = CompileShader(src.type, src.source);
		glAttachShader(program, shader);
		shader_objs.push_back(shader);
	}

	glLinkProgram(program);
	glValidateProgram(program);

	for (GLuint shader : shader_objs)
	{
		glDeleteShader(shader);
	}

	return program;

}

void Shader::SetMat4(const char* name, const glm::mat4 value) const
{
	Bind();
	auto location = glGetUniformLocation(m_ID, name);
	lcall(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)));
}

void Shader::SetIntArray(const char* name, int* arr, int length)
{
	Bind();
	auto location = glGetUniformLocation(m_ID, name);
	lcall(glUniform1iv(location, length, arr));
}

void Shader::SetVec3(const char* name, glm::vec3 vec)
{
	Bind();
	auto location = glGetUniformLocation(m_ID, name);
	lcall(glUniform3f(location, vec.x, vec.y, vec.z));
}

void Shader::SetIVec3(const char* name, glm::ivec3 vec)
{
	Bind();
	auto location = glGetUniformLocation(m_ID, name);
	lcall(glUniform3i(location, vec.x, vec.y, vec.z));
}


void Shader::SetVec4(const char* name, glm::vec4 vec) const
{
	Bind();
	auto location = glGetUniformLocation(m_ID, name);
	lcall(glUniform4f(location, vec.x, vec.y, vec.z, vec.w));
}
void Shader::SetVec2(const char* name, glm::vec2 vec) const
{
	Bind();
	auto location = glGetUniformLocation(m_ID, name);
	lcall(glUniform2f(location, vec.x, vec.y));
}
void Shader::SetFloat(const char* name, float a) const
{
	Bind();
	auto location = glGetUniformLocation(m_ID, name);
	lcall(glUniform1f(location, a));
}

void Shader::SetInt(const char* name, int a) const
{
	Bind();
	auto location = glGetUniformLocation(m_ID, name);
	lcall(glUniform1i(location, a));
}





