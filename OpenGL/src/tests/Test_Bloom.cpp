#include "Test_Bloom.h"

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

#define SHADOWMAP_WIDTH 1024
#define SHADOWMAP_HEIGHT 1024

namespace test
{
	float Test_Bloom::s_FOV = DEFAULT_FOV;
	float Test_Bloom::s_FOVMin = 1.f;
	float Test_Bloom::s_FOVMax = 90.f;

	Test_Bloom::Test_Bloom()
		: m_bEnableBloom(true)
		, m_Exposure(1.f)
		, m_CameraOrbitRadius(3.f)
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
			0.f,  0.f,  0.f, 0.f, 0.f, 1.f, 0.f, 0.f, // 1
			0.5f, 0.f,  0.f, 0.f, 0.f, 1.f, 1.f, 0.f, // 2
			0.5f, 0.5f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, // 3
			0.f,  0.5f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f  // 4
		};

		unsigned int planeIndices[] = {
			0, 1, 3,
			3, 1, 2
		};

		float cubeVertices[] = {
			// ---Begin: Top---
			0.f,  0.5f,  0.5f,  0.f,  1.f,  0.f, 0.f, 0.f, // 0
			0.5f, 0.5f,  0.5f,  0.f,  1.f,  0.f, 1.f, 0.f, // 1
			0.5f, 0.5f,  0.f,   0.f,  1.f,  0.f, 1.f, 1.f, // 2
			0.f,  0.5f,  0.f,   0.f,  1.f,  0.f, 0.f, 1.f, // 3
			// ---Begin: Front---
			0.f,  0.f,   0.5f,  0.f,  0.f,  1.f, 0.f, 0.f, // 4
			0.5f, 0.f,   0.5f,  0.f,  0.f,  1.f, 1.f, 0.f, // 5
			0.5f, 0.5f,  0.5f,  0.f,  0.f,  1.f, 1.f, 1.f, // 6
			0.f,  0.5f,  0.5f,  0.f,  0.f,  1.f, 0.f, 1.f, // 7
			// ---Begin: Left---
			0.f,  0.f,   0.f,  -1.f,  0.f,  0.f, 0.f, 0.f, // 8
			0.f,  0.f,   0.5f, -1.f,  0.f,  0.f, 1.f, 0.f, // 9
			0.f,  0.5f,  0.5f, -1.f,  0.f,  0.f, 1.f, 1.f, // 10
			0.f,  0.5f,  0.f,  -1.f,  0.f,  0.f, 0.f, 1.f, // 11
			// ---Begin: Back---
			0.5f, 0.f,   0.f,   0.f,  0.f, -1.f, 0.f, 0.f, // 12
			0.f,  0.f,   0.f,   0.f,  0.f, -1.f, 1.f, 0.f, // 13
			0.f,  0.5f,  0.f,   0.f,  0.f, -1.f, 1.f, 1.f, // 14
			0.5f, 0.5f,  0.f,   0.f,  0.f, -1.f, 0.f, 1.f, // 15
			// ---Begin: Right---
			0.5f, 0.f,   0.5f,  1.f,  0.f,  0.f, 0.f, 0.f, // 16 
			0.5f, 0.f,   0.f,   1.f,  0.f,  0.f, 1.f, 0.f, // 17
			0.5f, 0.5f,  0.f,   1.f,  0.f,  0.f, 1.f, 1.f, // 18
			0.5f, 0.5f,  0.5f,  1.f,  0.f,  0.f, 0.f, 1.f, // 19
			// ---Begin: Bottom---
			0.f,  0.f,   0.f,   0.f, -1.f,  0.f, 0.f, 0.f, // 20
			0.5f, 0.f,   0.f,   0.f, -1.f,  0.f, 1.f, 0.f, // 21
			0.5f, 0.f,   0.5f,  0.f, -1.f,  0.f, 1.f, 1.f, // 22
			0.f,  0.f,   0.5f,  0.f, -1.f,  0.f, 0.f, 1.f  // 23
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
			{0.3f, 0.f, 0.5f},
			{-0.75f, 0.8f, 1.f},
			{1.5f, 0.2f, 2.2f},
			{-1.2f, 0.1f, 1.5f}
		};

		m_PointLightPositions = {
			{0.7f, 0.7f, 3.5f},
			{-2.3f, 1.f, 1.3f},
			{-0.8f, 1.3f, -1.f}
		};
		
		m_PointLightColors = {
			{10.f, 0.f, 0.f},
			{0.f, 5.f, 0.f},
			{0.f, 0.f, 15.f}
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
		m_PlaneVBO.reset(new VertexBuffer(planeVertices, 32 * sizeof(float)));
		VertexBufferLayout planeLayout;
		planeLayout.Push<float>(3);
		planeLayout.Push<float>(3);
		planeLayout.Push<float>(2);
		m_PlaneVAO->AddBuffer(*m_PlaneVBO, planeLayout);
		m_PlaneIBO.reset(new IndexBuffer(planeIndices, 6));

		m_CubeVAO.reset(new VertexArray());
		m_CubeVBO.reset(new VertexBuffer(cubeVertices, 192 * sizeof(float)));
		VertexBufferLayout cubeLayout;
		cubeLayout.Push<float>(3);
		cubeLayout.Push<float>(3);
		cubeLayout.Push<float>(2);
		m_CubeVAO->AddBuffer(*m_CubeVBO, cubeLayout);
		m_CubeIBO.reset(new IndexBuffer(cubeIndices, 36));

		m_CubeShader.reset(new Shader("res/shaders/Cube_MRT.shader"));
		m_CubeShader->Bind();
		m_CubeShader->SetUniform1f("u_Material.shininess", 128.f);

		for (unsigned int i = 0; i < m_PointLightPositions.size(); ++i)
		{
			m_CubeShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].color", m_PointLightColors[i].x, m_PointLightColors[i].y, m_PointLightColors[i].z);
			m_CubeShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].position", m_PointLightPositions[i].x, m_PointLightPositions[i].y, m_PointLightPositions[i].z);
			m_CubeShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].ambientIntensity", 0.05f, 0.05f, 0.05f);
			m_CubeShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].diffuseIntensity", 0.6f, 0.6f, 0.6f);
			m_CubeShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].specularIntensity", 1.f, 1.f, 1.f);
			m_CubeShader->SetUniform1f("u_PointLights[" + std::to_string(i) + "].q", 0.032f);
			m_CubeShader->SetUniform1f("u_PointLights[" + std::to_string(i) + "].l", 0.09f);
			m_CubeShader->SetUniform1f("u_PointLights[" + std::to_string(i) + "].c", 1.f);
		}

		m_CubeDiffuseTexture.reset(new Texture("res/textures/Logo_D.png"));
		m_CubeShader->SetUniform1i("u_Material.diffuseTex", 0);
		m_CubeSpecularTexture.reset(new Texture("res/textures/Logo_S.png"));
		m_CubeShader->SetUniform1i("u_Material.specularTex", 1);

		m_PointLightShader.reset(new Shader("res/shaders/BasicColor_MRT.shader"));

		m_QuadVAO.reset(new VertexArray());
		m_QuadVBO.reset(new VertexBuffer(quadVertices, 16 * sizeof(float)));
		VertexBufferLayout QuadLayout;
		QuadLayout.Push<float>(2);
		QuadLayout.Push<float>(2);
		m_QuadVAO->AddBuffer(*m_QuadVBO, QuadLayout);
		m_QuadIBO.reset(new IndexBuffer(planeIndices, 6));

		m_BlurShader.reset(new Shader("res/shaders/GaussianBlur.shader"));
		m_BlurShader->Bind();
		m_BlurShader->SetUniform1i("u_BrightScene", 0);
		
		m_BloomShader.reset(new Shader("res/shaders/Bloom.shader"));
		m_BloomShader->Bind();
		m_BloomShader->SetUniform1i("u_NormalScene", 0);
		m_BloomShader->SetUniform1i("u_BlurredScene", 1);

		// TODO: needs abstraction
		// Floating point framebuffer used for HDR and MRT (Multiple Render Targets)
		glCreateFramebuffers(1, &m_HDRFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFBO);
		// Create two floating point color buffers, one for normal rendering, the other for brightness threshold values
		glCreateTextures(GL_TEXTURE_2D, 2, m_SceneTextures);
		for (unsigned int i = 0; i < 2; ++i)
		{
			glTextureStorage2D(m_SceneTextures[i], 1, GL_RGB16F, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
			glTextureSubImage2D(m_SceneTextures[i], 0, 0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, GL_RGB, GL_FLOAT, nullptr);
			glTextureParameteri(m_SceneTextures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_SceneTextures[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_SceneTextures[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_SceneTextures[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_SceneTextures[i], 0);
		}
		// Create and attach depth buffer
		glCreateRenderbuffers(1, &m_RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

		// Tell OpenGL which color attachments we will use for rendering
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		// Make sure to unbind the framebuffer to not accidentally render to the wrong framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Framebuffers used for Gaussian Blur
		glCreateFramebuffers(2, m_PingPongFBO);
		glCreateTextures(GL_TEXTURE_2D, 2, m_PingPongTextures);
		for (unsigned int i = 0; i < 2; ++i)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_PingPongFBO[i]);
			glTextureStorage2D(m_PingPongTextures[i], 1, GL_RGB16F, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
			glTextureSubImage2D(m_PingPongTextures[i], 0, 0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, GL_RGB, GL_FLOAT, nullptr);
			glTextureParameteri(m_PingPongTextures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_PingPongTextures[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// Clamp to the edge as the blur filter would otherwise sample repeated texture values
			glTextureParameteri(m_PingPongTextures[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_PingPongTextures[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PingPongTextures[i], 0);

			ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		}

		// Make sure to unbind the framebuffer to not accidentally render to the wrong framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Enable wireframe display
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	void Test_Bloom::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;

		// 1. Render scene into floating point framebuffer
		// -----------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFBO);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
		m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
		{
			m_CubeDiffuseTexture->Bind();
			m_CubeSpecularTexture->Bind(1);

			m_CubeShader->Bind();
			m_CubeShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
			m_CubeShader->SetUniform3f("u_ViewPos", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);

			// For 2D plane, we do not want it to be culled
			glDisable(GL_CULL_FACE);
			// Render floor
			{
				glm::mat4 model_floor = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -0.3f, 0.f)) *
					glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)) *
					glm::scale(glm::mat4(1.f), glm::vec3(50.f)) *
					// Move floor to (0, 0, 0), put this at last
					glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, 0.f));
				m_CubeShader->SetUniformMat4f("u_Model", model_floor);

				renderer.Draw(*m_PlaneVAO, *m_PlaneIBO, *m_CubeShader);
			}
			if (m_bMotionOn)
			{
				m_CubeRotation += m_ModelRotSpeed * deltaTime;
				m_CubeRotation = m_CubeRotation > 360.f ? 0.f : m_CubeRotation;
			}
			glEnable(GL_CULL_FACE);
			// Render cubes
			{
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
			// Render PointLights
			{
				m_PointLightShader->Bind();
				m_PointLightShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);

				for (unsigned int i = 0; i < m_PointLightPositions.size(); ++i)
				{
					glm::mat4 model_PointLight = glm::translate(glm::mat4(1.f), m_PointLightPositions[i]) *
						glm::scale(glm::mat4(1.f), glm::vec3(0.2f));
					m_PointLightShader->SetUniformMat4f("u_Model", model_PointLight);
					m_PointLightShader->SetUniform3f("u_Color", m_PointLightColors[i].x, m_PointLightColors[i].y, m_PointLightColors[i].z);

					renderer.Draw(*m_CubeVAO, *m_CubeIBO, *m_PointLightShader);
				}
			}
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// 2. Blur bright fragments with two-pass Gaussian Blur
		// ----------------------------------------------------
		bool bHorizontal = true, bFirstIteration = true;
		unsigned int blurAmount = 10;
		m_BlurShader->Bind();
		// After we have obtained an HDR texture and an extracted brightness texture,
		// we first fill one of the ping-pong framebuffer with the brightness texture
		// and then blur the image 10 times (5 times horizontally and 5 times vertically).
		// Basically, we first blur the scene's texture in the first framebuffer,
		// then blur the first framebuffer's colorbuffer into the second frameabuffer
		// and then the second framebuffer's colorbuffer into the first and so on.
		// Maybe framebuffers are not cleared so that newly rendered result will be added up to it, ending up with 10 times Gaussian Blur
		for (unsigned int i = 0; i < blurAmount; ++i)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_PingPongFBO[bHorizontal]);
			m_BlurShader->SetUniform1i("u_bHorizontal", bHorizontal);
			// Bind texture of the other framebuffer (or the brightness texture for the first iteration)
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, bFirstIteration ? m_SceneTextures[1] : m_PingPongTextures[!bHorizontal]);
			// Render quad to the bound texture
			renderer.Draw(*m_QuadVAO, *m_QuadIBO, *m_BlurShader);
			bHorizontal = !bHorizontal;
			bFirstIteration = bFirstIteration ? false : false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. Render blurred floating point color to quad and tonemap HDR colors to clamped color range
		// --------------------------------------------------------------------------------------------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_SceneTextures[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_PingPongTextures[!bHorizontal]);
		m_BloomShader->Bind();
		m_BloomShader->SetUniform1i("u_bEnableBloom", m_bEnableBloom);
		m_BloomShader->SetUniform1f("u_Exposure", m_Exposure);

		// Render the final quad
		renderer.Draw(*m_QuadVAO, *m_QuadIBO, *m_BloomShader);

	}

	void Test_Bloom::OnImGuiRender()
	{
		ImGui::Text(u8"使用RMB，ALT+RMB，MMB和WSAD来变换相机视角！");
		ImGui::Text(u8"相机位置: (%.1f, %.1f, %.1f)", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
		ImGui::Text(u8"相机朝向: (%.1f, %.1f, %.1f)", m_CameraFront.x, m_CameraFront.y, m_CameraFront.z);

		if (ImGui::Button(u8"重置视角"))
		{
			ResetView();
		}

		ImGui::Checkbox(u8"观赏模式", &m_bMotionOn);
		ImGui::SliderFloat(u8"运动速度", &m_ModelRotSpeed, 10.f, 360.f);

		ImGui::Checkbox(u8"启用Bloom", &m_bEnableBloom);
		ImGui::SliderFloat(u8"曝光", &m_Exposure, 0.f, 1.f);

	}

	void Test_Bloom::ProcessInput(GLFWwindow* window, float deltaTime)
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

	void Test_Bloom::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), s_FOVMin, s_FOVMax);
	}

	void Test_Bloom::ResetView()
	{
		m_bMotionOn = false;
		m_CubeRotation = 0.f;

		m_CameraPos = DEFAULT_CAMERAPOS;
		m_CameraFront = DEFAULT_CAMERAFRONT;
		m_Yaw = DEFAULT_YAW;
		m_Pitch = DEFAULT_PITCH;
		s_FOV = DEFAULT_FOV;
	}

}
