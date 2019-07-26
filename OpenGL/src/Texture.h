#pragma once

#include <string>

#include "Renderer.h"

class Texture
{
public:
	Texture(const std::string& filePath);
	~Texture();

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }

	/** Bind a named texture to a texturing target with the specified slot. */
	void Bind(unsigned int slot = 0) const;
	/** Unbind textures from a texturing target. */
	void Unbind() const;

private:
	unsigned int m_RendererID;

	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;
};
