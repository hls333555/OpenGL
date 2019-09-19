#pragma once

#include "Test.h"

#include <memory>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"

namespace test
{
	class Test_Bloom : public Test
	{
	public:
		Test_Bloom();
		~Test_Bloom() {}

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;

		virtual void ProcessInput(GLFWwindow* window, float deltaTime) override;

	private:
		static void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset);

		void ResetView();

	private:
		std::unique_ptr<VertexArray> m_PlaneVAO;
		std::unique_ptr<VertexBuffer> m_PlaneVBO;
		std::unique_ptr<IndexBuffer> m_PlaneIBO;

		std::unique_ptr<VertexArray> m_CubeVAO;
		std::unique_ptr<VertexBuffer> m_CubeVBO;
		std::unique_ptr<IndexBuffer> m_CubeIBO;

		std::unique_ptr<VertexArray> m_QuadVAO;
		std::unique_ptr<VertexBuffer> m_QuadVBO;
		std::unique_ptr<IndexBuffer> m_QuadIBO;

		std::unique_ptr<Shader> m_CubeShader;
		std::unique_ptr<Texture> m_CubeDiffuseTexture;
		std::unique_ptr<Texture> m_CubeSpecularTexture;

		std::vector<glm::vec3> m_CubePositions;

		std::vector<glm::vec3> m_PointLightPositions;
		std::vector<glm::vec3> m_PointLightColors;
		std::unique_ptr<Shader> m_PointLightShader;

		GLuint m_HDRFBO, m_SceneTextures[2], m_RBO;
		GLuint m_PingPongFBO[2], m_PingPongTextures[2];

		std::unique_ptr<Shader> m_BlurShader, m_BloomShader;

		bool m_bEnableBloom;
		float m_Exposure;

		glm::mat4 m_Proj, m_View;

		float m_CameraOrbitRadius;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		float m_CameraMoveSpeed;
		float m_CameraRotSpeed;
		float m_Yaw, m_Pitch;
		static float s_FOV, s_FOVMin, s_FOVMax;
		double m_LastXPos = 0, m_LastYPos = 0;

		bool m_bMotionOn = false;
		float m_CubeRotation = 0.f;
		float m_ModelRotSpeed;

	};

}
