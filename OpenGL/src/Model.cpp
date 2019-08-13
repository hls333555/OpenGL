#include "Model.h"

#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include "Texture.h"

Model::Model(const std::string& filePath)
{
	LoadModel(filePath);
}

void Model::Draw(const Shader& shader)
{
	for (auto& mesh : m_Meshes)
	{
		mesh->Draw(shader);
	}
}

void Model::LoadModel(const std::string& filePath)
{
	Assimp::Importer importer;
	// Do not set aiProcess_FlipUVs flag since it is already done in texture loading stage, @see Texture::LoadTexture()
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	// For path: "C:/Models/Model.fx", this will return "C:/Models/" as directory
	m_Dir = filePath.substr(0, filePath.find_last_of("/") + 1);

	ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	// Process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_Meshes.push_back(ProcessMesh(mesh, scene));
	}

	// Then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

std::unique_ptr<Mesh> Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<std::shared_ptr<Texture>> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex;

		// Process vertex positions
		glm::vec3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.position = pos;

		// Process normals and texture coordinates
		glm::vec3 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		vertex.normal = normal;

		// Process texture coordinates
		if (mesh->mTextureCoords[0])
		{
			glm::vec2 texCoord(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			vertex.texCoord = texCoord;
		}
		else
		{
			vertex.texCoord = glm::vec2(0.f);
		}

		vertices.push_back(vertex);
	}

	// Process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}
	
	// Process material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		auto diffuseTextures = LoadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::Diffuse);
		textures.insert(textures.end(), diffuseTextures.begin(), diffuseTextures.end());
		auto specularTextures = LoadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::Specular);
		textures.insert(textures.end(), specularTextures.begin(), specularTextures.end());
	}

	return std::make_unique<Mesh>(this, mesh->mName.C_Str(), vertices, indices, textures);
}

std::vector<std::shared_ptr<Texture>> Model::LoadMaterialTextures(aiMaterial* material, aiTextureType type, TextureType texType)
{
	std::vector<std::shared_ptr<Texture>> textures;
	for (unsigned int i = 0; i < material->GetTextureCount(type); ++i)
	{
		aiString aiStr;
		material->GetTexture(type, i, &aiStr);

		auto str = UTF8ToDefault(std::string(aiStr.C_Str()));

		bool bSkip = false;
		for (const auto& loadedTexture : m_LoadedTextures)
		{
			// If current processed texture is already loaded, skip loading...
			if (loadedTexture->GetFilePath().c_str() == m_Dir + str)
			{
				textures.push_back(loadedTexture);
				bSkip = true;
				break;
			}
		}
		if (!bSkip)
		{
			std::shared_ptr<Texture> texture = std::make_shared<Texture>(m_Dir, str, texType);
			textures.push_back(texture);
			m_LoadedTextures.push_back(texture);
		}
	}
	
	return textures;
}
