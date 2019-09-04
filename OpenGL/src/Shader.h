#pragma once

#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

struct ShaderSources
{
	std::string vertexShaderSource;
	std::string geometryShaderSource;
	std::string fragmentShaderSource;
};

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
	void SetUniform1f(const std::string& name, float value);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

private:
	/** Read shaders from shader file. */
	void ParseShader(const std::string& filePath, ShaderSources& outShaderSources);
	int CreateShader(const ShaderSources& shaderSources);
	unsigned int CompileShader(unsigned int type, const std::string& source);

	int GetUniformLocation(const std::string& name) const;

private:
	unsigned int m_RendererID;

	std::string m_filePath;

	mutable std::unordered_map<std::string, int> m_uniformLocationCache;

};
