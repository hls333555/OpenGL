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
	unsigned int num[2] = { 0, 0 };
	shader.Bind();
	auto& s = const_cast<Shader&>(shader);
	// Disable specular calculation if no specular texture is assigned
	s.SetUniform1i("u_Material.bHasSpecular", 0);
	for (unsigned int i = 0; i < m_Textures.size(); ++i)
	{
		std::string type;
		switch (m_Textures[i]->GetType())
		{
		case TextureType::BaseColor:
			type = "diffuseTex";
			s.SetUniform1i("u_Material." + type + std::to_string(++num[0]), i);
			break;
		case TextureType::Specular:
			s.SetUniform1i("u_Material.bHasSpecular", true);
			type = "specularTex";
			s.SetUniform1i("u_Material." + type + std::to_string(++num[1]), i);
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

void Mesh::DrawPBR(const Shader& shader, const std::unique_ptr<Shader>& defaultShader)
{
	Renderer renderer;

	// The first 3 texture units are for PBR required maps
	// At least, assimp is guaranteed to load basecolor, normal and roughness textures if exists
	// The bound texture units are hard-coded in PBR shader file, you must ensure they match
	// Also note that you must set uniform bindings manually here if binding points are not specified in PBR shader file
	unsigned int b = 0x0;
	for (auto& texture : m_Textures)
	{
		switch (texture->GetType())
		{
		case TextureType::BaseColor:
			texture->Bind(3);
			b |= 0x1 << 4;
			break;
		case TextureType::Normal:
			texture->Bind(4);
			b |= 0x1 << 3;
			break;
		case TextureType::Metallic:
			texture->Bind(5);
			b |= 0x1 << 2;
			break;
		case TextureType::Roughness:
			texture->Bind(6);
			b |= 0x1 << 1;
			break;
		case TextureType::AO:
			texture->Bind(7);
			b |= 0x1;
			break;
		default:
		  	ASSERT(false);
		}
	}

	// Unbind texture units which current mesh does not have
	// to prevent current mesh from wrongly using last mesh's bound texture
	for (unsigned int i = 0; i < 5; ++i)
	{
		if (!(b >> (4 - i) & 0x1))
		{
			// Use default lighting shader if basecolor texture is not (successfully) loaded
			// Comment these lines to disable this feature
			if (i == 0)
			{
				ASSERT(defaultShader);
				renderer.Draw(*m_VAO, *m_IBO, *defaultShader);
				return;
			}
			glActiveTexture(GL_TEXTURE0 + 3 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

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
		// If the texture to be loaded is already loaded AND their types correspond, skip loading...
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
		std::shared_ptr<Texture> texture = std::make_shared<Texture>(path, type);
		if (texture->GetLoadResult())
		{
			// Note: If, for example, you assign the texture of metallic texture slot to the ao texture slot,
			// the size of m_Texture remains unchanged (this newly loaded texture will take place of the original ao texture),
			// but the size of loadedTextures will increase 1 since it will load the already loaded metallic texture again
			m_Textures.push_back(texture);
			loadedTextures.push_back(texture);
		}
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
