#pragma once

#include <string>
#include <unordered_map>

class Shader
{
public:
	Shader(const std::string& filePath);
	~Shader();

	/** Install(bind) the program object as part of current rendering state. */
	void Bind() const;
	/** Uninstall(unbind) the program object as part of current rendering state. */
	void Unbind() const;

	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);

private:
	/** Read shaders from shader file. */
	void ParseShader(const std::string& filePath, std::string& vertexShaderSource, std::string& fragmentShaderSource);
	int CreateShader(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
	unsigned int CompileShader(unsigned int type, const std::string& source);

	int GetUniformLocation(const std::string& name);

	unsigned int m_RendererID;

	std::string m_filePath;

	std::unordered_map<std::string, int> m_uniformLocationCache;

};
