#include "Mesh.h"

#include <algorithm>

#include "VertexBufferLayout.h"
#include "Renderer.h"
#include "Model.h"

Mesh::Mesh(Model* ownerModel, const std::string& name, std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> textures)
	: m_OwnerModel(ownerModel)
	, m_Name(name)
	, m_Vertices(vertices), m_Indices(indices), m_Textures(textures)
{
	Setup();
}

void Mesh::Draw(const Shader& shader)
{
	unsigned int num[2] = {0, 0};
	shader.Bind();
	// Disable specular calculation if no specular texture is assigned
	const_cast<Shader&>(shader).SetUniform1i("u_Material.bHasSpecular", 0);
	for (unsigned int i = 0; i < m_Textures.size(); ++i)
	{
		std::string type;
		switch (m_Textures[i]->GetType())
		{
		case TextureType::Diffuse:
			type = "diffuseTex";
			const_cast<Shader&>(shader).SetUniform1i("u_Material." + type + std::to_string(++num[0]), i);
			break;
		case TextureType::Specular:
			const_cast<Shader&>(shader).SetUniform1i("u_Material.bHasSpecular", 1);
			type = "specularTex";
			const_cast<Shader&>(shader).SetUniform1i("u_Material." + type + std::to_string(++num[1]), i);
			break;
		default:
			ASSERT(false);
			break;
		}
	}

	for (unsigned int i = 0; i < m_Textures.size(); ++i)
	{
		m_Textures[i]->Bind(i);
	}

	Renderer renderer;
	renderer.Draw(*m_VAO, *m_IBO, shader);
}

void Mesh::SetTexture(const std::string& path, TextureType type)
{
	// Remove the texture of the same type first if found
	auto it = std::find_if(m_Textures.begin(), m_Textures.end(), [type](std::shared_ptr<Texture> item) { return item->GetType() == type; });
	if (it != m_Textures.end())
	{
		m_Textures.erase(it);
	}

	auto& loadedTextures = m_OwnerModel->GetLoadedTextures();
	bool bSkip = false;
	for (const auto& loadedTexture : loadedTextures)
	{
		// If the texture to be loaded is already loaded AND their type corresponds, just add that to m_Textures
		if (loadedTexture->GetFilePath() == path && loadedTexture->GetType() == type)
		{
			m_Textures.push_back(loadedTexture);
			bSkip = true;
			break;
		}
	}
	// Otherwise, load it
	if (!bSkip)
	{
		m_Textures.push_back(std::make_shared<Texture>(path, type));
	}
}

void Mesh::Setup()
{
	m_VAO.reset(new VertexArray());

	m_VBO.reset(new VertexBuffer(&m_Vertices[0], m_Vertices.size() * sizeof(Vertex)));

	VertexBufferLayout layout;
	layout.Push<float>(3);
	layout.Push<float>(3);
	layout.Push<float>(2);
	m_VAO->AddBuffer(*m_VBO, layout);

	m_IBO.reset(new IndexBuffer(&m_Indices[0], m_Indices.size()));
}
