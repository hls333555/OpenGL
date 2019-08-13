#pragma once

#include <string>

#include "Renderer.h"

enum class TextureType
{
	None = -1,
	Diffuse,
	Normal,
	Specular,
};

class Texture
{
public:
	Texture(const std::string& filePath, TextureType type = TextureType::None);
	Texture(const std::string& mainFilePath, const std::string& subFilePath, TextureType type);
	~Texture();

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	inline const std::string& GetFilePath() const { return m_FilePath; }
	inline TextureType GetType() const { return m_Type; }

	/** Bind a named texture to a texturing target with the specified slot. */
	void Bind(unsigned int slot = 0) const;
	/** Unbind textures from a texturing target. */
	void Unbind() const;

private:
	void LoadTexture(const std::string& filePath);

private:
	unsigned int m_RendererID;

	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;

	TextureType m_Type;

};
