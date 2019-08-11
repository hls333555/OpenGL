#include "Test_Lighting.h"

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
	float Test_Lighting::s_FOV = DEFAULT_FOV;
	float Test_Lighting::s_FOVMin = 1.f;
	float Test_Lighting::s_FOVMax = 90.f;

	Test_Lighting::Test_Lighting()
		: m_LightPos(1.f, 1.f, 2.f)
		, m_CubeRotSpeed(90.f)
		, m_CameraOrbitRadius(3.f)
		, m_CameraPos(DEFAULT_CAMERAPOS)
		, m_CameraFront(DEFAULT_CAMERAFRONT)
		, m_CameraUp(glm::vec3(0.f, 1.f, 0.f))
		, m_CameraMoveSpeed(3.f)
		, m_CameraRotSpeed(15.f)
		, m_Yaw(DEFAULT_YAW), m_Pitch(DEFAULT_PITCH)
	{
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it is closer to the camera than the former one
		glDepthFunc(GL_LESS);

		//GLCALL(glEnable(GL_BLEND));
		// Set this to blend transparency properly
		//GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		// WORKAROUND: C style function pointer must take a static function if it is a member function!
		glfwSetScrollCallback(Test::s_Window, OnMouseScroll);

		m_CubePositions = {
			{0.f, 0.f, 0.f},
			{1.f, 1.5f, 0.75f},
			{0.4f, 2.3f, -1.2f},
			{1.3f, -0.5f, 0.35f},
			{-0.25f, 3.f, 2.2f},
			{-1.4f, -2.1f, 0.5f},
			{-1.7f, 1.3f, -1.6f},
			{1.1f, -0.8f, -2.1f},
			{-1.1f, -0.7f, -0.9f}
		};

		float vertices[] = {
			// Positions        // Normals      // TextureCoordinates
			// ---Begin: Top---
			0.f,  0.5f,  0.5f,  0.f, 1.f, 0.f,  0.f, 0.f, // 0
			0.5f, 0.5f,  0.5f,  0.f, 1.f, 0.f,  1.f, 0.f, // 1
			0.5f, 0.5f,  0.f,   0.f, 1.f, 0.f,  1.f, 1.f, // 2
			0.f,  0.5f,  0.f,   0.f, 1.f, 0.f,  0.f, 1.f, // 3
			// ---Begin: Front---
			0.f,  0.f,   0.5f,  0.f, 0.f, 1.f,  0.f, 0.f, // 4
			0.5f, 0.f,   0.5f,  0.f, 0.f, 1.f,  1.f, 0.f, // 5
			0.5f, 0.5f,  0.5f,  0.f, 0.f, 1.f,  1.f, 1.f, // 6
			0.f,  0.5f,  0.5f,  0.f, 0.f, 1.f,  0.f, 1.f, // 7
			// ---Begin: Left---
			0.f,  0.f,   0.f,  -1.f, 0.f, 0.f,  0.f, 0.f, // 8
			0.f,  0.f,   0.5f, -1.f, 0.f, 0.f,  1.f, 0.f, // 9
			0.f,  0.5f,  0.5f, -1.f, 0.f, 0.f,  1.f, 1.f, // 10
			0.f,  0.5f,  0.f,  -1.f, 0.f, 0.f,  0.f, 1.f, // 11
			// ---Begin: Back---
			0.f,  0.f,   0.f,   0.f, 0.f, -1.f, 0.f, 0.f, // 12
			0.5f, 0.f,   0.f,   0.f, 0.f, -1.f, 1.f, 0.f, // 13
			0.5f, 0.5f,  0.f,   0.f, 0.f, -1.f, 1.f, 1.f, // 14
			0.f,  0.5f,  0.f,   0.f, 0.f, -1.f, 0.f, 1.f, // 15
			// ---Begin: Right---
			0.5f, 0.f,   0.5f,  1.f, 0.f, 0.f,  0.f, 0.f, // 16 
			0.5f, 0.f,   0.f,   1.f, 0.f, 0.f,  1.f, 0.f, // 17
			0.5f, 0.5f,  0.f,   1.f, 0.f, 0.f,  1.f, 1.f, // 18
			0.5f, 0.5f,  0.5f,  1.f, 0.f, 0.f,  0.f, 1.f, // 19
			// ---Begin: Bottom---
			0.f,  0.f,   0.f,   0.f, -1.f, 0.f, 0.f, 0.f, // 20
			0.5f, 0.f,   0.f,   0.f, -1.f, 0.f, 1.f, 0.f, // 21
			0.5f, 0.f,   0.5f,  0.f, -1.f, 0.f, 1.f, 1.f, // 22
			0.f,  0.f,   0.5f,  0.f, -1.f, 0.f, 0.f, 1.f  // 23
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

		m_CubeVAO.reset(new VertexArray());

		m_VBO.reset(new VertexBuffer(vertices, 192 * sizeof(float)));

		VertexBufferLayout layout_Cube;
		layout_Cube.Push<float>(3);
		layout_Cube.Push<float>(3);
		layout_Cube.Push<float>(2);
		m_CubeVAO->AddBuffer(*m_VBO, layout_Cube);

		m_IBO.reset(new IndexBuffer(indices, 36));

		m_CubeShader.reset(new Shader("res/shaders/Cube_Lighting.shader"));
		m_CubeShader->Bind();
		m_CubeShader->SetUniform1f("u_Material.shininess", 64.f);
		m_CubeShader->SetUniform3f("u_DirLight.ambientIntensity", 0.1f, 0.1f, 0.1f);
		m_CubeShader->SetUniform3f("u_DirLight.diffuseIntensity", 1.f, 1.f, 1.f);
		m_CubeShader->SetUniform3f("u_DirLight.specularIntensity", 1.f, 1.f, 1.f);

		m_CubeDiffuseTexture.reset(new Texture("res/textures/Logo_D.png"));
		m_CubeShader->SetUniform1i("u_Material.diffuseTex", 0);
		m_CubeSpecularTexture.reset(new Texture("res/textures/Logo_S.png"));
		m_CubeShader->SetUniform1i("u_Material.specularTex", 1);

		m_LightSourceVAO.reset(new VertexArray());

		m_VBO->Bind();

		VertexBufferLayout layout_LightSource;
		layout_LightSource.Push<float>(3);
		layout_LightSource.Push<float>(3);
		layout_LightSource.Push<float>(2);
		m_LightSourceVAO->AddBuffer(*m_VBO, layout_LightSource);

		m_IBO->Bind();

		m_LightSourceShader.reset(new Shader("res/shaders/LightSource.shader"));
		m_LightSourceShader->Bind();

	}

	void Test_Lighting::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;
		m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
		m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
		// Render cube
		{
			if (bMotionOn)
			{
				m_CubeMotionRotation += m_CubeRotSpeed * deltaTime;
				m_CubeMotionRotation = m_CubeMotionRotation > 360.f ? 0.f : m_CubeMotionRotation;
			}

			m_CubeShader->Bind();
			m_CubeShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
			//m_CubeShader->SetUniform3f("u_DirLight.position", m_LightPos.x, m_LightPos.y, m_LightPos.z);
			m_CubeShader->SetUniform3f("u_DirLight.direction", -0.2f, -1.f, -0.3f);
			m_CubeShader->SetUniform3f("u_ViewPos", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);

			m_CubeDiffuseTexture->Bind();
			m_CubeSpecularTexture->Bind(1);

			for (unsigned int i = 0; i < m_CubePositions.size(); ++i)
			{
				m_CubeInitialRotation = i * 20.f;
				glm::mat4 model_Cube = glm::translate(glm::mat4(1.f), m_CubePositions[i]) *
					glm::rotate(glm::mat4(1.f), glm::radians(m_CubeInitialRotation), glm::vec3(1.f, 0.3f, 0.5f)) *
					// Motion rotation
					glm::rotate(glm::mat4(1.f), glm::radians(m_CubeMotionRotation), glm::vec3(0.f, 1.f, 0.f)) *
					// Move cube to (0, 0, 0), put this at last
					glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, -0.25f));

				m_CubeShader->SetUniformMat4f("u_Model", model_Cube);

				renderer.Draw(*m_CubeVAO, *m_IBO, *m_CubeShader);
			}

		}
		// Render light source
		{
			glm::mat4 model_LightSource = glm::translate(glm::mat4(1.f), m_LightPos) * glm::scale(glm::mat4(1.f), glm::vec3(0.2f));

			m_LightSourceShader->Bind();
			m_LightSourceShader->SetUniformMat4f("u_Model", model_LightSource);
			m_LightSourceShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);

			renderer.Draw(*m_LightSourceVAO, *m_IBO, *m_LightSourceShader);
		}

	}

	void Test_Lighting::OnImGuiRender()
	{
		ImGui::Text("Use RMB, ALT+RMB and WSAD to transform the camera!");
		ImGui::Checkbox("Motion", &bMotionOn);
		ImGui::SliderFloat("Motion Speed", &m_CubeRotSpeed, 10.f, 360.f);
		if (ImGui::Button("Reset View"))
		{
			ResetView();
		}
		ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
		ImGui::Text("Camera Front: (%.1f, %.1f, %.1f)", m_CameraFront.x, m_CameraFront.y, m_CameraFront.z);

	}

	void Test_Lighting::ProcessInput(GLFWwindow* window, float deltaTime)
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

	void Test_Lighting::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), s_FOVMin, s_FOVMax);
	}

	void Test_Lighting::ResetView()
	{
		bMotionOn = false;
		m_CubeMotionRotation = 0.f;

		m_CameraPos = DEFAULT_CAMERAPOS;
		m_CameraFront = DEFAULT_CAMERAFRONT;
		m_Yaw = DEFAULT_YAW;
		m_Pitch = DEFAULT_PITCH;
		s_FOV = DEFAULT_FOV;
	}

}
