#include "Test_FaceCulling.h"

#include "VertexBufferLayout.h"

#include "imgui/imgui.h"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

// Put it at last!
#include <GLFW/glfw3.h>

#define DEFAULT_CAMERAFRONT glm::vec3(0.f, 0.f, -1.f)
#define DEFAULT_CAMERAPOS	DEFAULT_CAMERAFRONT * -m_CameraOrbitRadius
#define DEFAULT_YAW			-90.f
#define DEFAULT_PITCH		0.f
#define DEFAULT_FOV			45.f

namespace test
{
	float Test_FaceCulling::s_FOV = DEFAULT_FOV;
	float Test_FaceCulling::s_FOVMin = 1.f;
	float Test_FaceCulling::s_FOVMax = 90.f;

	Test_FaceCulling::Test_FaceCulling()
		: m_CameraOrbitRadius(3.f)
		, m_CameraPos(DEFAULT_CAMERAPOS)
		, m_CameraFront(DEFAULT_CAMERAFRONT)
		, m_CameraUp(glm::vec3(0.f, 1.f, 0.f))
		, m_CameraMoveSpeed(3.f)
		, m_CameraRotSpeed(15.f)
		, m_Yaw(DEFAULT_YAW), m_Pitch(DEFAULT_PITCH)
		, m_ModelRotSpeed(90.f)
	{
		// WORKAROUND: C style function pointer must take a static function if it is a member function!
		glfwSetScrollCallback(Test::s_Window, OnMouseScroll);

		float planeVertices[] = {
			0.f,  0.f,   0.f, 0.f, 0.f, // 1
			0.5f, 0.f,   0.f, 1.f, 0.f, // 2
			0.5f, 0.5f,  0.f, 1.f, 1.f, // 3
			0.f,  0.5f,  0.f, 0.f, 1.f, // 4
		};

		unsigned int planeIndices[] = {
			1, 2, 4,
			4, 2, 3
		};

		float cubeVertices[] = {
			// ---Begin: Top---
			0.f,  0.5f,  0.5f, 0.f, 0.f, // 0
			0.5f, 0.5f,  0.5f, 1.f, 0.f, // 1
			0.5f, 0.5f,  0.f,  1.f, 1.f, // 2
			0.f,  0.5f,  0.f,  0.f, 1.f, // 3
			// ---Begin: Front---
			0.f,  0.f,   0.5f, 0.f, 0.f, // 4
			0.5f, 0.f,   0.5f, 1.f, 0.f, // 5
			0.5f, 0.5f,  0.5f, 1.f, 1.f, // 6
			0.f,  0.5f,  0.5f, 0.f, 1.f, // 7
			// ---Begin: Left---
			0.f,  0.f,   0.f,  0.f, 0.f, // 8
			0.f,  0.f,   0.5f, 1.f, 0.f, // 9
			0.f,  0.5f,  0.5f, 1.f, 1.f, // 10
			0.f,  0.5f,  0.f,  0.f, 1.f, // 11
			// ---Begin: Back---
			0.5f, 0.f,   0.f,  0.f, 0.f, // 12
			0.f,  0.f,   0.f,  1.f, 0.f, // 13
			0.f,  0.5f,  0.f,  1.f, 1.f, // 14
			0.5f, 0.5f,  0.f,  0.f, 1.f, // 15
			// ---Begin: Right---
			0.5f, 0.f,   0.5f, 0.f, 0.f, // 16 
			0.5f, 0.f,   0.f,  1.f, 0.f, // 17
			0.5f, 0.5f,  0.f,  1.f, 1.f, // 18
			0.5f, 0.5f,  0.5f, 0.f, 1.f, // 19
			// ---Begin: Bottom---
			0.f,  0.f,   0.f,  0.f, 0.f, // 20
			0.5f, 0.f,   0.f,  1.f, 0.f, // 21
			0.5f, 0.f,   0.5f, 1.f, 1.f, // 22
			0.f,  0.f,   0.5f, 0.f, 1.f  // 23
		};

		// Make sure vertices are draw in counter-clockwise order for default face culling to work properly
		unsigned int cubeIndices[] = {
			// ---Begin: Top---
			0,  1,  3,
			3,  1,  2,
			// ---Begin: Front---
			4,  5,  7,
			7,  5,  6,
			// ---Begin: Left---
			8,  9,  11,
			11, 9,  10,
			// ---Begin: Back---
			12, 13, 15,
			15, 13, 14,
			// ---Begin: Right---
			16, 17, 19,
			19, 17, 18,
			// ---Begin: Bottom---
			20, 21, 23,
			23, 21, 22
		};

		GLCALL(glEnable(GL_BLEND));
		// Set this to blend transparency properly
		GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it is closer to the camera than the former one
		glDepthFunc(GL_LESS);

		m_PlaneVAO.reset(new VertexArray());
		m_PlaneVBO.reset(new VertexBuffer(planeVertices, 20 * sizeof(float)));
		VertexBufferLayout planeLayout;
		planeLayout.Push<float>(3);
		planeLayout.Push<float>(2);
		m_PlaneVAO->AddBuffer(*m_PlaneVBO, planeLayout);
		m_PlaneIBO.reset(new IndexBuffer(planeIndices, 6));

		m_FloorShader.reset(new Shader("res/shaders/BasicColor.shader"));
		m_FloorShader->Bind();
		m_FloorShader->SetUniform4f("u_Color", 1.f, 1.f, 1.f, 1.f);

		m_LogoShader.reset(new Shader("res/shaders/BasicTexture.shader"));
		m_LogoTexture.reset(new Texture("res/textures/Logo_Trans_D.png"));
		m_LogoShader->Bind();
		m_LogoShader->SetUniform1i("u_Texture", 0);

		m_CubeVAO.reset(new VertexArray());
		m_CubeVBO.reset(new VertexBuffer(cubeVertices, 120 * sizeof(float)));
		VertexBufferLayout cubeLayout;
		cubeLayout.Push<float>(3);
		cubeLayout.Push<float>(2);
		m_CubeVAO->AddBuffer(*m_CubeVBO, cubeLayout);
		m_CubeIBO.reset(new IndexBuffer(cubeIndices, 36));

		m_CubeShader.reset(new Shader("res/shaders/Cube.shader"));

	}

	void Test_FaceCulling::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;
		m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
		m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
		// Render floor
		{
			glm::mat4 model_floor = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -0.3f, 0.f)) *
				glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)) *
				glm::scale(glm::mat4(1.f), glm::vec3(10.f)) *
				// Move floor to (0, 0, 0), put this at last
				glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, 0.f));
			m_FloorShader->Bind();
			m_FloorShader->SetUniformMat4f("u_Model", model_floor);
			m_FloorShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);

			renderer.Draw(*m_PlaneVAO, *m_PlaneIBO, *m_FloorShader);
		}
		if (bMotionOn)
		{
			m_CubeRotation += m_ModelRotSpeed * deltaTime;
			m_CubeRotation = m_CubeRotation > 360.f ? 0.f : m_CubeRotation;
		}
		glEnable(GL_CULL_FACE);
		// Render cube
		{
			m_CubeShader->Bind();
			m_CubeShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
			glm::mat4 model_Cube = glm::rotate(glm::mat4(1.f), glm::radians(m_CubeRotation), glm::vec3(0.f, 1.f, 0.f)) *
				// Move cube to (0, 0, 0), put this at last
				glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, -0.25f));
			m_CubeShader->SetUniformMat4f("u_Model", model_Cube);

			renderer.Draw(*m_CubeVAO, *m_CubeIBO, *m_CubeShader);
		}
		// For 2D plane, we do not want it to be culled
		glDisable(GL_CULL_FACE);
		// Render grass
		{
			m_LogoTexture->Bind();
			m_LogoShader->Bind();
			m_LogoShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
			glm::mat4 model_Grass = glm::translate(glm::mat4(1.f), glm::vec3(-1.f, 0.f, 0.f)) *
				// Move cube to (0, 0, 0), put this at last
				glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, -0.25f));
			m_LogoShader->SetUniformMat4f("u_Model", model_Grass);

			renderer.Draw(*m_PlaneVAO, *m_PlaneIBO, *m_LogoShader);
		}

	}

	void Test_FaceCulling::OnImGuiRender()
	{
		ImGui::Text(u8"使用RMB，ALT+RMB，MMB和WSAD来变换相机视角！");
		ImGui::Text(u8"相机位置: (%.1f, %.1f, %.1f)", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
		ImGui::Text(u8"相机朝向: (%.1f, %.1f, %.1f)", m_CameraFront.x, m_CameraFront.y, m_CameraFront.z);

		if (ImGui::Button(u8"重置视角"))
		{
			ResetView();
		}

		ImGui::Checkbox(u8"观赏模式", &bMotionOn);
		ImGui::SliderFloat(u8"运动速度", &m_ModelRotSpeed, 10.f, 360.f);

	}

	void Test_FaceCulling::ProcessInput(GLFWwindow* window, float deltaTime)
	{
		////////////////////////////////////////////////////////////
		// Camera: Focus Object (Center) ///////////////////////////
		////////////////////////////////////////////////////////////

		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		{
			m_CameraPos = glm::vec3(0.f) - m_CameraFront * m_CameraOrbitRadius;
		}

		////////////////////////////////////////////////////////////
		// Camera: Planar Movement /////////////////////////////////
		////////////////////////////////////////////////////////////

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			m_CameraPos += m_CameraFront * m_CameraMoveSpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			m_CameraPos -= m_CameraFront * m_CameraMoveSpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			m_CameraPos += glm::normalize(glm::cross(m_CameraUp, m_CameraFront)) * m_CameraMoveSpeed * deltaTime;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			m_CameraPos -= glm::normalize(glm::cross(m_CameraUp, m_CameraFront)) * m_CameraMoveSpeed * deltaTime;
		}

		////////////////////////////////////////////////////////////
		// Camera: Angular Movement ////////////////////////////////
		////////////////////////////////////////////////////////////

		static bool bFirstPress = true;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
		{
			// When right mouse button is pressed, hide cursor and constrain it at the original location
			glfwSetInputMode(Test::s_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			double xpos = 0, ypos = 0;
			glfwGetCursorPos(Test::s_Window, &xpos, &ypos);

			if (bFirstPress)
			{
				m_LastXPos = xpos;
				m_LastYPos = ypos;
				bFirstPress = false;
			}

			float xOffset = float(xpos - m_LastXPos) * m_CameraRotSpeed * deltaTime;
			float yOffset = float(m_LastYPos - ypos) * m_CameraRotSpeed * deltaTime;
			m_LastXPos = xpos;
			m_LastYPos = ypos;

			m_Yaw += xOffset;
			m_Pitch = glm::clamp(m_Pitch + yOffset, -89.f, 89.f);

			glm::vec3 front;
			front.x = cos(glm::radians(m_Pitch)) * cos(glm::radians(m_Yaw));
			front.y = sin(glm::radians(m_Pitch));
			front.z = cos(glm::radians(m_Pitch)) * sin(glm::radians(m_Yaw));
			m_CameraFront = glm::normalize(front);

			if (glfwGetKey(Test::s_Window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(Test::s_Window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
			{
				// Orbit around the center object
				m_CameraPos = m_CameraFront * -1.f * m_CameraOrbitRadius;
			}
		}
		else
		{
			bFirstPress = true;
			// When right mouse button is rleased, restore cursor behavior
			glfwSetInputMode(Test::s_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

	}

	void Test_FaceCulling::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), s_FOVMin, s_FOVMax);
	}

	void Test_FaceCulling::ResetView()
	{
		bMotionOn = false;
		m_CubeRotation = 0.f;

		m_CameraPos = DEFAULT_CAMERAPOS;
		m_CameraFront = DEFAULT_CAMERAFRONT;
		m_Yaw = DEFAULT_YAW;
		m_Pitch = DEFAULT_PITCH;
		s_FOV = DEFAULT_FOV;
	}

}
