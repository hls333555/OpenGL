#pragma once

#include "Test.h"

#include <memory>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Model.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace test
{
	class Test_BasicInstancing : public Test
	{
	public:
		Test_BasicInstancing();
		~Test_BasicInstancing() {}

		virtual void OnRender() override;

	private:
		std::unique_ptr<VertexArray> m_VAO;
		std::unique_ptr<VertexBuffer> m_VBO;
		/** Instanced arrays */
		std::unique_ptr<VertexBuffer> m_TranslationsVBO;
		std::unique_ptr<IndexBuffer> m_IBO;
		std::unique_ptr<Shader> m_Shader;

	};

	class Test_AdvancedInstancing : public Test
	{
		struct RockTransform
		{
			float Angle;
			float offsetX, offsetZ;
			glm::mat4 rot, scale;
		};

	public:
		Test_AdvancedInstancing();
		~Test_AdvancedInstancing() {}

		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;

		virtual void ProcessInput(GLFWwindow* window, float deltaTime) override;

	private:
		static void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset);

		void ResetView();

	private:
		std::unique_ptr<Model> m_PlanetModel;
		std::unique_ptr<Shader> m_ModelShader;
		std::unique_ptr<Model> m_RockModel;

		bool m_bMotionOn = false;
		float m_PlanetMotionRotation = 0.f;
		float m_PlanetRotSpeed;
#define ROCK_AMOUNT 1000
		float m_RockOrbitRadius;
		float m_RockOrbitSpeed;
		/** Offsets will be randomized within [-m_RandomOffsetBound, m_RandomOffsetBound] */
		float m_RandomOffsetBound;

		glm::mat4 m_Proj, m_View;

		RockTransform m_RockTransforms[ROCK_AMOUNT];
		glm::mat4 m_RockMatrices[ROCK_AMOUNT];

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
