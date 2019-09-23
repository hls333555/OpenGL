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
	class Test_BasicPBR : public Test
	{
	public:
		Test_BasicPBR();
		~Test_BasicPBR() {}

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;

		virtual void ProcessInput(GLFWwindow* window, float deltaTime) override;

	private:
		static void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset);

		void ResetView();

	private:
		std::unique_ptr<VertexArray> m_VAO;
		std::unique_ptr<VertexBuffer> m_VBO;
		std::unique_ptr<IndexBuffer> m_IBO;
		std::unique_ptr<Shader> m_Shader;

		bool m_bUseTexture = true;
		std::unique_ptr<Texture> m_BaseColorTexture;
		//std::unique_ptr<Texture> m_NormalTexture;
		std::unique_ptr<Texture> m_RoughnessTexture;
		std::unique_ptr<Texture> m_MetallicTexture;

		std::unique_ptr<Shader> m_PointLightShader;
		std::vector<glm::vec3>* m_PointLightPositions;
		std::vector<glm::vec3> m_PointLightPositions1;
		std::vector<glm::vec3> m_PointLightPositions2;
		std::vector<glm::vec3>* m_PointLightColors;
		std::vector<glm::vec3> m_PointLightColors1;
		std::vector<glm::vec3> m_PointLightColors2;

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

	};

	class Test_IBLPBR : public Test
	{
	public:
		Test_IBLPBR();
		~Test_IBLPBR() {}

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;

		virtual void ProcessInput(GLFWwindow* window, float deltaTime) override;

	private:
		static void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset);

		void ResetView();

	private:
		std::unique_ptr<VertexArray> m_VAO;
		std::unique_ptr<VertexBuffer> m_VBO;
		std::unique_ptr<IndexBuffer> m_IBO;

		std::unique_ptr<VertexArray> m_BackgroundVAO;
		std::unique_ptr<VertexBuffer> m_BackgroundVBO;

		std::unique_ptr<VertexArray> m_QuadVAO;
		std::unique_ptr<VertexBuffer> m_QuadVBO;
		std::unique_ptr<IndexBuffer> m_QuadIBO;

		std::unique_ptr<Shader> m_PointLightShader;
		std::vector<glm::vec3> m_PointLightPositions;
		std::vector<glm::vec3> m_PointLightColors;

		GLuint m_CaptureFBO, m_CaptureRBO;
		GLuint m_HDRTexture, m_EnvCubemap, m_IrradianceMap;
		GLuint m_PrefilterMap, m_BRDFLUTTexture;

		std::unique_ptr<Shader> m_PBRShader, m_BackgroundShader, m_EquirectangularToCubemapShader, m_IrradianceShader;
		std::unique_ptr<Shader> m_PrefilterShader, m_BRDFShader;

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

	};

}
