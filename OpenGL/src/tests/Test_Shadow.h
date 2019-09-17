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
	class Test_DirectionalShadowMapping : public Test
	{
	public:
		Test_DirectionalShadowMapping();
		~Test_DirectionalShadowMapping() {}

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

		std::unique_ptr<Shader> m_CubeShader;
		std::unique_ptr<Texture> m_CubeDiffuseTexture;
		std::unique_ptr<Texture> m_CubeSpecularTexture;

		std::vector<glm::vec3> m_CubePositions;

		GLuint m_DepthMapFBO, m_DepthTextureBuffer;
		std::unique_ptr<Shader> m_DepthShader;

		glm::mat4 m_Proj, m_View;

		glm::vec3 m_DirLightPos;

		float m_CameraOrbitRadius;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		float m_CameraMoveSpeed;
		float m_CameraRotSpeed;
		float m_Yaw, m_Pitch;
		static float s_FOV, s_FOVMin, s_FOVMax;
		double m_LastXPos = 0, m_LastYPos = 0;

		bool bMotionOn = false;
		float m_CubeRotation = 0.f;
		float m_ModelRotSpeed;

	};

	class Test_OmniDirectionalShadowMapping : public Test
	{
	public:
		Test_OmniDirectionalShadowMapping();
		~Test_OmniDirectionalShadowMapping() {}

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

		std::unique_ptr<Shader> m_CubeShader;
		std::unique_ptr<Texture> m_CubeDiffuseTexture;
		std::unique_ptr<Texture> m_CubeSpecularTexture;

		std::vector<glm::vec3> m_CubePositions;

		GLuint m_DepthMapFBO, m_DepthCubemapBuffer;
		std::unique_ptr<Shader> m_DepthShader;

		glm::mat4 m_Proj, m_View;

		glm::vec3 m_PointLightPos;

		float m_CameraOrbitRadius;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		float m_CameraMoveSpeed;
		float m_CameraRotSpeed;
		float m_Yaw, m_Pitch;
		static float s_FOV, s_FOVMin, s_FOVMax;
		double m_LastXPos = 0, m_LastYPos = 0;

		bool bMotionOn = false;
		float m_CubeRotation = 0.f;
		float m_ModelRotSpeed;

	};

}
