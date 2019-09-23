#include "Shader.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "Renderer.h"

Shader::Shader(const std::string& filePath)
	: m_filePath(filePath)
	, m_RendererID(0)
{
	ShaderSources shaderSources;
	ParseShader(filePath, shaderSources);
	m_RendererID = CreateShader(shaderSources);
}

Shader::~Shader()
{
	// Delete the program object
	GLCALL(glDeleteProgram(m_RendererID));
}

void Shader::Bind() const
{
	GLCALL(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
	GLCALL(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& name, int value)
{
	GLCALL(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform1f(const std::string& name, float value)
{
	GLCALL(glUniform1f(GetUniformLocation(name), value));
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
	GLCALL(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
	GLCALL(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
	GLCALL(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

void Shader::ParseShader(const std::string& filePath, ShaderSources& outShaderSources)
{
	std::ifstream stream(filePath);

	enum class ShaderType
	{
		NONE = -1,
		VERTEX = 0,
		GEOMETRY = 1,
		FRAGMENT = 2
	};

	std::string line;
	std::stringstream ss[3];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				type = ShaderType::VERTEX;
			}
			else if (line.find("geometry") != std::string::npos)
			{
				type = ShaderType::GEOMETRY;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			ss[(int)type] << line << "\n";
		}
	}
	outShaderSources.vertexShaderSource = ss[0].str();
	outShaderSources.geometryShaderSource = ss[1].str();
	outShaderSources.fragmentShaderSource = ss[2].str();
}
// TODO: use for-loops to optimize
int Shader::CreateShader(const ShaderSources& shaderSources)
{
	// Create the program object
	GLCALL(unsigned int program = glCreateProgram());

	unsigned int vs = CompileShader(GL_VERTEX_SHADER, shaderSources.vertexShaderSource);
	unsigned int gs;
	if (shaderSources.geometryShaderSource != "")
	{
		gs = CompileShader(GL_GEOMETRY_SHADER, shaderSources.geometryShaderSource);
	}
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, shaderSources.fragmentShaderSource);

	// Attach shaders to the program object
	GLCALL(glAttachShader(program, vs));
	if (shaderSources.geometryShaderSource != "")
	{
		GLCALL(glAttachShader(program, gs));
	}
	GLCALL(glAttachShader(program, fs));
	// Link the program
	GLCALL(glLinkProgram(program));
	// Check to see whether the executables contained in program can execute given the current OpenGL state.
	GLCALL(glValidateProgram(program));

	GLCALL(glDeleteShader(vs));
	if (shaderSources.geometryShaderSource != "")
	{
		GLCALL(glDeleteShader(gs));
	}
	GLCALL(glDeleteShader(fs));

	int result;
	GLCALL(glGetProgramiv(program, GL_VALIDATE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GLCALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length));
		// Allocate on the stack dynamically
		char* message = (char*)_malloca(length * sizeof(char));
		// Return the information log for the shader object
		GLCALL(glGetProgramInfoLog(program, length, &length, message));
		std::cout << "Failed to validate program!" << std::endl;
		std::cout << message << std::endl;

		return 0;
	}

	return program;
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
	// Create a shader object
	GLCALL(unsigned int id = glCreateShader(type));
	// Convert string to a char pointer
	const char* src = source.c_str();
	// Load the source code into the shader object
	GLCALL(glShaderSource(id, 1, &src, nullptr));
	// Compile the shader object
	GLCALL(glCompileShader(id));

	int result;
	GLCALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GLCALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		// Allocate on the stack dynamically
		char* message = (char*)_malloca(length * sizeof(char));
		// Return the information log for the shader object
		GLCALL(glGetShaderInfoLog(id, length, &length, message));
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragmemt") << " shader!" << std::endl;
		std::cout << message << std::endl;
		GLCALL(glDeleteShader(id));
		return 0;
	}

	return id;
}

int Shader::GetUniformLocation(const std::string& name) const
{
	if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end())
	{
		return m_uniformLocationCache[name];
	}

	GLCALL(unsigned int location = glGetUniformLocation(m_RendererID, name.c_str()));
	// If "name" is not used in the fragment shader, it will return -1
	if (location == -1)
	{
		std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
	}
	m_uniformLocationCache[name] = location;
	return location;
}
