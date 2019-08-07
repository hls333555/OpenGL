#pragma once

#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

class Shader
{
public:
	Shader(const std::string& filePath);
	~Shader();

	/** Install(bind) the program object as part of current rendering state. */
	void Bind() const;
	/** Uninstall(unbind) program objects. */
	void Unbind() const;

	void SetUniform1i(const std::string& name, int value);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

private:
	/** Read shaders from shader file. */
	void ParseShader(const std::string& filePath, std::string& vertexShaderSource, std::string& fragmentShaderSource);
	int CreateShader(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
	unsigned int CompileShader(unsigned int type, const std::string& source);

	int GetUniformLocation(const std::string& name) const;

private:
	unsigned int m_RendererID;

	std::string m_filePath;

	mutable std::unordered_map<std::string, int> m_uniformLocationCache;

};
