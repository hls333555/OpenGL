#include "Test_Cube.h"

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
	float Test_Cube::s_FOV = DEFAULT_FOV;

	Test_Cube::Test_Cube()
		: m_CameraOrbitRadius(3.f)
		, m_CameraPos(DEFAULT_CAMERAPOS)
		, m_CameraFront(DEFAULT_CAMERAFRONT)
		, m_CameraUp(glm::vec3(0.f, 1.f, 0.f))
		, m_CameraMoveSpeed(3.f)
		, m_CameraRotSpeed(15.f)
		, m_Yaw(DEFAULT_YAW), m_Pitch(DEFAULT_PITCH)
		, m_FOVMin(1.f), m_FOVMax(45.f)
		, m_ModelRotSpeed(90.f)
	{
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it is closer to the camera than the former one
		glDepthFunc(GL_LESS);

		GLCALL(glEnable(GL_BLEND));
		// Set this to blend transparency properly
		GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		// WORKAROUND: C style function pointer must take a static function if it is a member function!
		glfwSetScrollCallback(Test::s_Window, OnMouseScroll);

		float vertices[] = {
			// ---Begin: Top---
			0.f,  0.5f, -0.5f, 0.f, 0.f, // 0
			0.5f, 0.5f, -0.5f, 1.f, 0.f, // 1
			0.5f, 0.5f,  0.f,  1.f, 1.f, // 2
			0.f,  0.5f,  0.f,  0.f, 1.f, // 3
			// ---Begin: Front---
			0.f,  0.f,  -0.5f, 0.f, 0.f, // 4
			0.5f, 0.f,  -0.5f, 1.f, 0.f, // 5
			0.5f, 0.5f, -0.5f, 1.f, 1.f, // 6
			0.f,  0.5f, -0.5f, 0.f, 1.f, // 7
			// ---Begin: Left---
			0.f,  0.f,   0.f,  0.f, 0.f, // 8
			0.f,  0.f,  -0.5f, 1.f, 0.f, // 9
			0.f,  0.5f, -0.5f, 1.f, 1.f, // 10
			0.f,  0.5f,  0.f,  0.f, 1.f, // 11
			// ---Begin: Back---
			0.f,  0.f,   0.f,  0.f, 0.f, // 12
			0.5f, 0.f,   0.f,  1.f, 0.f, // 13
			0.5f, 0.5f,  0.f,  1.f, 1.f, // 14
			0.f,  0.5f,  0.f,  0.f, 1.f, // 15
			// ---Begin: Right---
			0.5f, 0.f,  -0.5f, 0.f, 0.f, // 16 
			0.5f, 0.f,   0.f,  1.f, 0.f, // 17
			0.5f, 0.5f,  0.f,  1.f, 1.f, // 18
			0.5f, 0.5f, -0.5f, 0.f, 1.f, // 19
			// ---Begin: Bottom---
			0.f,  0.f,   0.f,  0.f, 0.f, // 20
			0.5f, 0.f,   0.f,  1.f, 0.f, // 21
			0.5f, 0.f,  -0.5f, 1.f, 1.f, // 22
			0.f,  0.f,  -0.5f, 0.f, 1.f  // 23
		};

		unsigned int indices[] = {
			// ---Begin: Top---
			0,  1,  2,
			2,  3,  0,
			// ---Begin: Front---
			4,  5,  6,
			6,  7,  4,
			// ---Begin: Left---
			8,  9,  10,
			10, 11, 8,
			// ---Begin: Back---
			12, 13, 14,
			14, 15, 12,
			// ---Begin: Right---
			16, 17, 18,
			18, 19, 16,
			// ---Begin: Bottom---
			20, 21, 22,
			22, 23, 20
		};

		m_VAO.reset(new VertexArray());

		m_VBO.reset(new VertexBuffer(vertices, 120 * sizeof(float)));

		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(2);
		m_VAO->AddBuffer(*m_VBO, layout);

		m_IBO.reset(new IndexBuffer(indices, 36));

		m_Shader.reset(new Shader("res/shaders/Cube.shader"));
		m_Shader->Bind();

		m_Texture.reset(new Texture("res/textures/Logo.png"));
		m_Texture->Bind();
		m_Shader->SetUniform1i("u_Texture", 0);

	}

	void Test_Cube::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;

		{
			if (bMotionOn)
			{
				m_ModelRotation += m_ModelRotSpeed * deltaTime;
				m_ModelRotation = m_ModelRotation > 360.f ? 0.f : m_ModelRotation;
			}
			glm::mat4 model = glm::rotate(glm::mat4(1.f), glm::radians(m_ModelRotation), glm::vec3(0.f, 1.f, 0.f)) *
				// Move cube to (0, 0, 0), put this at last
				glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, 0.25f));
			m_Shader->SetUniformMat4f("u_Model", model);
		}
		{
			m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
			m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
			m_Shader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
		}

		renderer.Draw(*m_VAO, *m_IBO, *m_Shader);

	}

	void Test_Cube::OnImGuiRender()
	{
		ImGui::Text("Use RMB, ALT+RMB and WSAD to transform the camera!");
		ImGui::Checkbox("Motion", &bMotionOn);
		ImGui::SliderFloat("Motion Speed", &m_ModelRotSpeed, 10.f, 360.f);
		if (ImGui::Button("Reset View"))
		{
			ResetView();
		}
		ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
		ImGui::Text("Camera Front: (%.1f, %.1f, %.1f)", m_CameraFront.x, m_CameraFront.y, m_CameraFront.z);

	}

	void Test_Cube::ProcessInput(GLFWwindow* window, float deltaTime)
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

	void Test_Cube::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), 1.f, 45.f);
	}

	void Test_Cube::ResetView()
	{
		bMotionOn = false;
		m_ModelRotation = 0.f;

		m_CameraPos = DEFAULT_CAMERAPOS;
		m_CameraFront = DEFAULT_CAMERAFRONT;
		m_Yaw = DEFAULT_YAW;
		m_Pitch = DEFAULT_PITCH;
		s_FOV = DEFAULT_FOV;
	}

}
