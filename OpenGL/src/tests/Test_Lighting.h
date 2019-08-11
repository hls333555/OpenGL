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
	class Test_Lighting : public Test
	{
	public:
		Test_Lighting();
		~Test_Lighting() {}

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;

		virtual void ProcessInput(GLFWwindow* window, float deltaTime) override;

	private:
		static void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset);

		void ResetView();

	private:
		std::unique_ptr<VertexArray> m_CubeVAO;
		std::unique_ptr<VertexBuffer> m_VBO;
		std::unique_ptr<IndexBuffer> m_IBO;
		std::unique_ptr<Shader> m_CubeShader;
		std::unique_ptr<Texture> m_CubeDiffuseTexture;
		std::unique_ptr<Texture> m_CubeSpecularTexture;

		std::unique_ptr<VertexArray> m_PointLightVAO;
		std::unique_ptr<Shader> m_PointLightShader;
		
		glm::mat4 m_Proj, m_View;
		glm::vec3 m_LightPos;

		std::vector<glm::vec3> m_CubePositions;
		float m_CubeInitialRotation = 0.f;
		bool bMotionOn = false;
		float m_CubeMotionRotation = 0.f;
		float m_CubeRotSpeed;

		std::vector<glm::vec3> m_PointLightPositions;
		bool m_bEnableDirLight = true;
		bool m_bEnablePointLights = true;
		bool m_bEnableSpotLight = true;

		float m_CameraOrbitRadius;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		float m_CameraMoveSpeed;
		float m_CameraRotSpeed;
		float m_Yaw, m_Pitch;
		static float s_FOV, s_FOVMin, s_FOVMax;
		double m_LastXPos = 0, m_LastYPos = 0;

	};

}
