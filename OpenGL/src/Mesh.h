#pragma once

#include <string>
#include <memory>
#include <vector>

#include "glm/glm.hpp"

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"

class Model;

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

class Mesh
{
public:
	Mesh(Model* ownerModel, const std::string& name, std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> textures);

	inline const std::string& GetName() const { return m_Name; }
	inline const std::vector<std::shared_ptr<Texture>>& GetTextures() const { return m_Textures; }
	inline const std::unique_ptr<VertexArray>& GetVAO() const { return m_VAO; }
	inline const std::unique_ptr<IndexBuffer>& GetIBO() const { return m_IBO; }

	/** Used for normal rendering (non-PBR). */
	void Draw(const Shader& shader);
	/** Used for PBR rendering. */
	void DrawPBR(const Shader& shader, const std::unique_ptr<Shader>& defaultShader);

	void SetTexture(const std::string& path, TextureType type);

private:
	void Setup();

private:
	std::string m_Name;
	std::unique_ptr<VertexArray> m_VAO;
	std::unique_ptr<VertexBuffer> m_VBO;
	std::unique_ptr<IndexBuffer> m_IBO;

	std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_Indices;
	std::vector<std::shared_ptr<Texture>> m_Textures;

	Model* m_OwnerModel;

};
