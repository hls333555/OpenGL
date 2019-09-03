#include "Test_FrameBuffer.h"

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
	float Test_FrameBuffer::s_FOV = DEFAULT_FOV;
	float Test_FrameBuffer::s_FOVMin = 1.f;
	float Test_FrameBuffer::s_FOVMax = 90.f;

	Test_FrameBuffer::Test_FrameBuffer()
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
			0.f,  0.f,  0.f, 0.f, 0.f, // 1
			0.5f, 0.f,  0.f, 1.f, 0.f, // 2
			0.5f, 0.5f, 0.f, 1.f, 1.f, // 3
			0.f,  0.5f, 0.f, 0.f, 1.f  // 4
		};

		unsigned int planeIndices[] = {
			0, 1, 3,
			3, 1, 2
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

		m_CubePositions = {
			{0.f, 0.f, 0.f},
			{0.5f, 0.f, -0.5f}
		};

		float quadVertices[] = {
			-1.f, -1.f, 0.f, 0.f, // 1
			 1.f, -1.f, 1.f, 0.f, // 2
			 1.f,  1.f, 1.f, 1.f, // 3
			-1.f,  1.f, 0.f, 1.f  // 4
		};

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

		m_CubeVAO.reset(new VertexArray());
		m_CubeVBO.reset(new VertexBuffer(cubeVertices, 120 * sizeof(float)));
		VertexBufferLayout cubeLayout;
		cubeLayout.Push<float>(3);
		cubeLayout.Push<float>(2);
		m_CubeVAO->AddBuffer(*m_CubeVBO, cubeLayout);
		m_CubeIBO.reset(new IndexBuffer(cubeIndices, 36));

		m_CubeShader.reset(new Shader("res/shaders/Cube.shader"));

		m_QuadVAO.reset(new VertexArray());
		m_QuadVBO.reset(new VertexBuffer(quadVertices, 16 * sizeof(float)));
		VertexBufferLayout QuadLayout;
		QuadLayout.Push<float>(2);
		QuadLayout.Push<float>(2);
		m_QuadVAO->AddBuffer(*m_QuadVBO, QuadLayout);
		m_QuadIBO.reset(new IndexBuffer(planeIndices, 6));

		m_ScreenShader.reset(new Shader("res/shaders/BasicScreenTexture.shader"));
		m_ScreenShader->Bind();
		m_ScreenShader->SetUniform1i("u_ScreenTexture", 0);

		// TODO: needs abstraction
		// Create framebuffer
		glCreateFramebuffers(1, &m_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

		// Generate a color attachment texture
		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureBuffer);
		glTextureParameteri(m_TextureBuffer, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_TextureBuffer, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// It will crash here if internalformat is set to GL_RGB
		glTextureStorage2D(m_TextureBuffer, 1, GL_RGB8, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
		glTextureSubImage2D(m_TextureBuffer, 0, 0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
		// Attach the texture to currently bound framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureBuffer, 0);

		// Create a renderbuffer for depth and stencil attachment since we won't be sampling these
		glCreateRenderbuffers(1, &m_RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		// Attach the renderbuffer to currently bound framebuffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

		// Check if the framebuffer is actually complete
		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		// Make sure to unbind the framebuffer to not accidentally render to the wrong framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Enable wireframe display
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	void Test_FrameBuffer::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;

		// First pass
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		glEnable(GL_DEPTH_TEST);
		//glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Draw scene to the texture
		{
			m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
			m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
			// For 2D plane, we do not want it to be culled
			glDisable(GL_CULL_FACE);
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
				for (unsigned int i = 0; i < m_CubePositions.size(); ++i)
				{
					glm::mat4 model_Cube = glm::translate(glm::mat4(1.f), m_CubePositions[i]) *
						glm::rotate(glm::mat4(1.f), glm::radians(m_CubeRotation), glm::vec3(0.f, 1.f, 0.f)) *
						// Move cube to (0, 0, 0), put this at last
						glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, -0.25f));
					m_CubeShader->SetUniformMat4f("u_Model", model_Cube);

					renderer.Draw(*m_CubeVAO, *m_CubeIBO, *m_CubeShader);
				}
			}
			glDisable(GL_CULL_FACE);
		}

		// Second pass
		// Back to default frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		//glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		// Render texture as a scene
		{
			glBindTexture(GL_TEXTURE_2D, m_TextureBuffer);

			renderer.Draw(*m_QuadVAO, *m_QuadIBO, *m_ScreenShader);
		}

	}

	void Test_FrameBuffer::OnImGuiRender()
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

	void Test_FrameBuffer::ProcessInput(GLFWwindow* window, float deltaTime)
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

	void Test_FrameBuffer::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), s_FOVMin, s_FOVMax);
	}

	void Test_FrameBuffer::ResetView()
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
