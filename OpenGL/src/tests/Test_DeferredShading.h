#pragma once

#include "Test.h"

#include <memory>

#include "Model.h"
#include "Shader.h"

namespace test
{
	class Test_DeferredShading : public Test
	{
	public:
		Test_DeferredShading();
		~Test_DeferredShading() {}

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;

		virtual void ProcessInput(GLFWwindow* window, float deltaTime) override;

	private:
		static void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset);

		void ResetView();

	private:
		std::unique_ptr<Model> m_Model;
		std::unique_ptr<Shader> m_GeometryPassShader;
		std::unique_ptr<Shader> m_LightingPassShader;

		std::unique_ptr<VertexArray> m_QuadVAO;
		std::unique_ptr<VertexBuffer> m_QuadVBO;
		std::unique_ptr<IndexBuffer> m_QuadIBO;

		bool m_bMotionOn = false;
		float m_ModelMotionRotation = 0.f;
		float m_ModelRotSpeed;
		float m_ModelScale;

		glm::mat4 m_Proj, m_View;

		GLuint m_GBuffer, m_GPositionTexture, m_GNormalTexture, m_GColorSpecTexture, m_RBO;

		std::unique_ptr<VertexArray> m_PointLightVAO;
		std::unique_ptr<VertexBuffer> m_PointLightVBO;
		std::unique_ptr<IndexBuffer> m_PointLightIBO;
		std::unique_ptr<Shader> m_PointLightShader;
		std::vector<glm::vec3> m_PointLightPositions;
		std::vector<glm::vec3> m_PointLightColors;

		glm::vec3 m_AmbientIntensity;
		glm::vec3 m_DiffuseIntensity;
		glm::vec3 m_SpecularIntensity;

		bool m_bDrawPointLights = true;

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
