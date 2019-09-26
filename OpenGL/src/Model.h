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
	std::vector<std::shared_ptr<Texture>>& GetLoadedTextures() { return m_LoadedTextures; }
	const std::string& GetFullDir() const { return m_FullDir; }

	void Draw(const Shader& shader, bool bPBR = false, const std::unique_ptr<Shader>& defaultShader = nullptr);

private:
	void LoadModel(const std::string& filePath);

	void ProcessNode(aiNode* node, const aiScene* scene);
	std::unique_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<std::shared_ptr<Texture>> LoadMaterialTextures(aiMaterial* material, aiTextureType type, TextureType texType);
	void LoadTextureFromDisk(const std::string& str, std::vector<std::shared_ptr<Texture>>& textures, TextureType texType);

private:
	std::vector<std::unique_ptr<Mesh>> m_Meshes;

	std::vector<std::shared_ptr<Texture>> m_LoadedTextures;

	std::string m_Dir, m_FullDir;

};
