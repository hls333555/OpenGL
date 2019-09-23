#include "Texture.h"

#include "stb_image/stb_image.h"

Texture::Texture(const std::string& filePath, TextureType type, bool bFlipOnLoad)
	: m_RendererID(0)
	, m_FilePath(filePath)
	, m_LocalBuffer(nullptr)
	, m_Width(0)
	, m_Height(0)
	, m_BPP(0)
	, m_Type(type)
{
	LoadTexture(filePath, bFlipOnLoad);
}

Texture::Texture(const std::string& mainFilePath, const std::string& subFilePath, TextureType type, bool bFlipOnLoad)
	: m_RendererID(0)
	, m_FilePath(mainFilePath + subFilePath)
	, m_LocalBuffer(nullptr)
	, m_Width(0)
	, m_Height(0)
	, m_BPP(0)
	, m_Type(type)
{
	LoadTexture(mainFilePath + subFilePath, bFlipOnLoad);
}

void Texture::LoadTexture(const std::string& filePath, bool bFlipOnLoad)
{
	// OpenGL expects the texture pixels to start at the bottom-left(0,0) instead of the top-left
	// Typically, when png image is being loaded, it is stored in scanlines from the top to the bottom of the image, so we need to flip it on load
	stbi_set_flip_vertically_on_load(bFlipOnLoad);
	m_LocalBuffer = stbi_load(filePath.c_str(), &m_Width, &m_Height, &m_BPP, 4/*RGBA*/);

	// Generate texture names
	GLCALL(glGenTextures(1, &m_RendererID));

	// Bind
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_RendererID));

	// Set texture parameters
	// This is the minification filter that how the texture will be resampled down if it needs to be rendered smaller per pixel
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	// Set this to GL_CALMP_TO_EDGE will cause issues sometimes
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	// Send OpenGL the texture data
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8/*8-bits per channel*/, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));
	// Unbind
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));

	if (m_LocalBuffer)
	{
		// Free the local buffer
		stbi_image_free(m_LocalBuffer);
	}
}

Texture::~Texture()
{
	// Delete named textures
	GLCALL(glDeleteTextures(1, &m_RendererID));
}

void Texture::Bind(unsigned int slot) const
{
	// Select active texture unit(slot)
	GLCALL(glActiveTexture(GL_TEXTURE0 + slot));
	GLCALL(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() const
{
	GLCALL(glBindTexture(GL_TEXTURE_2D, 0));
}
