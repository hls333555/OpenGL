#pragma once

#include "Mesh.h"

#include "assimp/scene.h"

#include "Shader.h"
#include "Texture.h"

class Model
{
public:
	Model(const std::string& filePath);

	const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const { return m_Meshes; }
	const std::vector<std::shared_ptr<Texture>>& GetLoadedTextures() const { return m_LoadedTextures; }

	void Draw(const Shader& shader);

private:
	void LoadModel(const std::string& filePath);

	void ProcessNode(aiNode* node, const aiScene* scene);
	std::unique_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<std::shared_ptr<Texture>> LoadMaterialTextures(aiMaterial* material, aiTextureType type, TextureType texType);

private:
	std::vector<std::unique_ptr<Mesh>> m_Meshes;

	std::vector<std::shared_ptr<Texture>> m_LoadedTextures;

	std::string m_Dir;

};
