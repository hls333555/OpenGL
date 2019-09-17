#include "Test_Shadow.h"

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
	float Test_DirectionalShadowMapping::s_FOV = DEFAULT_FOV;
	float Test_DirectionalShadowMapping::s_FOVMin = 1.f;
	float Test_DirectionalShadowMapping::s_FOVMax = 90.f;

	Test_DirectionalShadowMapping::Test_DirectionalShadowMapping()
		: m_DirLightPos(glm::vec3(-2.f, 4.f, -1.f))
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
			{-0.75f, 0.8f, 1.f}
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

		m_CubeShader.reset(new Shader("res/shaders/DirectionalLightingWithShadows.shader"));
		m_CubeShader->Bind();
		m_CubeShader->SetUniform1f("u_Material.shininess", 128.f);

		m_CubeShader->SetUniform3f("u_DirLight.ambientIntensity", 0.15f, 0.15f, 0.15f);
		m_CubeShader->SetUniform3f("u_DirLight.diffuseIntensity", 0.5f, 0.5f, 0.5f);
		m_CubeShader->SetUniform3f("u_DirLight.specularIntensity", 0.5f, 0.5f, 0.5f);

		m_CubeDiffuseTexture.reset(new Texture("res/textures/Logo_D.png"));
		m_CubeShader->SetUniform1i("u_Material.diffuseTex", 0);
		m_CubeSpecularTexture.reset(new Texture("res/textures/Logo_S.png"));
		m_CubeShader->SetUniform1i("u_Material.specularTex", 1);
		m_CubeShader->SetUniform1i("u_ShadowMap", 2);

		m_DepthShader.reset(new Shader("res/shaders/DepthMap.shader"));

		// TODO: needs abstraction
		glCreateFramebuffers(1, &m_DepthMapFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthTextureBuffer);
		glTextureParameteri(m_DepthTextureBuffer, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_DepthTextureBuffer, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// Used to address over sampling issue
		glTextureParameteri(m_DepthTextureBuffer, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(m_DepthTextureBuffer, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.f, 1.f, 1.f, 1.f };
		glTextureParameterfv(m_DepthTextureBuffer, GL_TEXTURE_BORDER_COLOR, borderColor);

		// It will crash here if internalformat is set to GL_DEPTH_COMPONENT - explicit size must be specified
		glTextureStorage2D(m_DepthTextureBuffer, 1, GL_DEPTH_COMPONENT16, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
		glTextureSubImage2D(m_DepthTextureBuffer, 0, 0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTextureBuffer, 0);
		// We only need the depth information when rendering the scene from the light's perspective
		// so there is no need for a color buffer
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		// Make sure to unbind the framebuffer to not accidentally render to the wrong framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Enable wireframe display
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	void Test_DirectionalShadowMapping::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;

		// First pass - render to depth map
		glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		// This is necessary to fix some shadow artifacts!
		glCullFace(GL_FRONT);
		// Draw scene to the texture
		{
			m_Proj = glm::ortho(-10.f, 10.f, -10.f, 10.f, 1.f, 7.5f);
			// Render from directional light's point of view
			m_View = glm::lookAt(m_DirLightPos, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

			// For 2D plane, we do not want it to be culled
			glDisable(GL_CULL_FACE);
			// Render floor
			{
				glm::mat4 model_floor = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -0.3f, 0.f)) *
					glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)) *
					glm::scale(glm::mat4(1.f), glm::vec3(50.f)) *
					// Move floor to (0, 0, 0), put this at last
					glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, 0.f));
				m_DepthShader->Bind();
				m_DepthShader->SetUniformMat4f("u_Model", model_floor);
				m_DepthShader->SetUniformMat4f("u_LightSpaceViewProjection", m_Proj * m_View);

				renderer.Draw(*m_PlaneVAO, *m_PlaneIBO, *m_DepthShader);
			}
			if (bMotionOn)
			{
				m_CubeRotation += m_ModelRotSpeed * deltaTime;
				m_CubeRotation = m_CubeRotation > 360.f ? 0.f : m_CubeRotation;
			}
			glEnable(GL_CULL_FACE);
			// Render cube
			{
				for (unsigned int i = 0; i < m_CubePositions.size(); ++i)
				{
					glm::mat4 model_Cube = glm::translate(glm::mat4(1.f), m_CubePositions[i]) *
						glm::rotate(glm::mat4(1.f), glm::radians(m_CubeRotation), glm::vec3(0.f, 1.f, 0.f)) *
						// Move cube to (0, 0, 0), put this at last
						glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, -0.25f));
					m_DepthShader->SetUniformMat4f("u_Model", model_Cube);

					renderer.Draw(*m_CubeVAO, *m_CubeIBO, *m_DepthShader);
				}
			}
			glCullFace(GL_BACK);
			glDisable(GL_CULL_FACE);
		}

		// Second pass - render scene as normal with shadow mapping (using depth map)
		glViewport(0.f, 0.f, WINDOW_WIDTH, WINDOW_HEIGHT);
		// Back to default frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Render scene
		{
			m_CubeShader->Bind();
			m_CubeShader->SetUniformMat4f("u_LightSpaceViewProjection", m_Proj * m_View);

			m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
			m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);

			m_CubeShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
			
			m_CubeShader->SetUniform3f("u_ViewPos", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);

			m_CubeShader->SetUniform3f("u_DirLight.direction", -m_DirLightPos.x, -m_DirLightPos.y, -m_DirLightPos.z);

			m_CubeDiffuseTexture->Bind();
			m_CubeSpecularTexture->Bind(1);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, m_DepthTextureBuffer);

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
			if (bMotionOn)
			{
				m_CubeRotation += m_ModelRotSpeed * deltaTime;
				m_CubeRotation = m_CubeRotation > 360.f ? 0.f : m_CubeRotation;
			}
			glEnable(GL_CULL_FACE);
			// Render cube
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
			glDisable(GL_CULL_FACE);
		}

	}

	void Test_DirectionalShadowMapping::OnImGuiRender()
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

	void Test_DirectionalShadowMapping::ProcessInput(GLFWwindow* window, float deltaTime)
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

	void Test_DirectionalShadowMapping::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), s_FOVMin, s_FOVMax);
	}

	void Test_DirectionalShadowMapping::ResetView()
	{
		bMotionOn = false;
		m_CubeRotation = 0.f;

		m_CameraPos = DEFAULT_CAMERAPOS;
		m_CameraFront = DEFAULT_CAMERAFRONT;
		m_Yaw = DEFAULT_YAW;
		m_Pitch = DEFAULT_PITCH;
		s_FOV = DEFAULT_FOV;
	}


	float Test_OmniDirectionalShadowMapping::s_FOV = DEFAULT_FOV;
	float Test_OmniDirectionalShadowMapping::s_FOVMin = 1.f;
	float Test_OmniDirectionalShadowMapping::s_FOVMax = 90.f;

	Test_OmniDirectionalShadowMapping::Test_OmniDirectionalShadowMapping()
		: m_PointLightPos(glm::vec3(1.f, 3.f, -2.f))
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

		m_CubeShader.reset(new Shader("res/shaders/PointLightingWithShadows.shader"));
		m_CubeShader->Bind();
		m_CubeShader->SetUniform1f("u_Material.shininess", 128.f);

		m_CubeShader->SetUniform3f("u_PointLight.position", m_PointLightPos.x, m_PointLightPos.y, m_PointLightPos.z);
		m_CubeShader->SetUniform3f("u_PointLight.ambientIntensity", 0.15f, 0.15f, 0.15f);
		m_CubeShader->SetUniform3f("u_PointLight.diffuseIntensity", 0.6f, 0.6f, 0.6f);
		m_CubeShader->SetUniform3f("u_PointLight.specularIntensity", 1.f, 1.f, 1.f);
		m_CubeShader->SetUniform1f("u_PointLight.q", 0.032f);
		m_CubeShader->SetUniform1f("u_PointLight.l", 0.09f);
		m_CubeShader->SetUniform1f("u_PointLight.c", 1.f);

		m_CubeDiffuseTexture.reset(new Texture("res/textures/Logo_D.png"));
		m_CubeShader->SetUniform1i("u_Material.diffuseTex", 0);
		m_CubeSpecularTexture.reset(new Texture("res/textures/Logo_S.png"));
		m_CubeShader->SetUniform1i("u_Material.specularTex", 1);
		m_CubeShader->SetUniform1i("u_ShadowCubemap", 2);

		m_DepthShader.reset(new Shader("res/shaders/DepthCubemap.shader"));

		// TODO: needs abstraction
		glCreateFramebuffers(1, &m_DepthMapFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_DepthCubemapBuffer);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthCubemapBuffer);
		glTextureParameteri(m_DepthCubemapBuffer, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_DepthCubemapBuffer, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_DepthCubemapBuffer, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_DepthCubemapBuffer, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_DepthCubemapBuffer, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemapBuffer, 0);
		// We only need the depth information when rendering the scene from the light's perspective
		// so there is no need for a color buffer
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		// Make sure to unbind the framebuffer to not accidentally render to the wrong framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Enable wireframe display
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	void Test_OmniDirectionalShadowMapping::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;

		float near = 1.f, far = 25.f;

		// First pass - render to depth map
		glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		//glCullFace(GL_FRONT);
		// Draw scene to the texture
		{
			float aspect = (float)SHADOWMAP_WIDTH / (float)SHADOWMAP_HEIGHT;
			m_Proj = glm::perspective(glm::radians(90.f), aspect, near, far);
			// Render from point light's point of view
			std::vector<glm::mat4> shadowTransforms{
				m_Proj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
				m_Proj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)),
				m_Proj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
				m_Proj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)),
				m_Proj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)),
				m_Proj * glm::lookAt(m_PointLightPos, m_PointLightPos + glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f))
			};

			// For 2D plane, we do not want it to be culled
			glDisable(GL_CULL_FACE);
			// Render floor
			{
				glm::mat4 model_floor = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -0.3f, 0.f)) *
					glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)) *
					glm::scale(glm::mat4(1.f), glm::vec3(50.f)) *
					// Move floor to (0, 0, 0), put this at last
					glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, 0.f));
				m_DepthShader->Bind();
				m_DepthShader->SetUniformMat4f("u_Model", model_floor);
				for (unsigned int i = 0; i < 6; ++i)
				{
					m_DepthShader->SetUniformMat4f("u_ShadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
				}
				m_DepthShader->SetUniform1f("u_FarPlane", far);
				m_DepthShader->SetUniform3f("u_LightPos", m_PointLightPos.x, m_PointLightPos.y, m_PointLightPos.z);

				renderer.Draw(*m_PlaneVAO, *m_PlaneIBO, *m_DepthShader);
			}
			if (bMotionOn)
			{
				m_CubeRotation += m_ModelRotSpeed * deltaTime;
				m_CubeRotation = m_CubeRotation > 360.f ? 0.f : m_CubeRotation;
			}
			glEnable(GL_CULL_FACE);
			// Render cube
			{
				for (unsigned int i = 0; i < m_CubePositions.size(); ++i)
				{
					glm::mat4 model_Cube = glm::translate(glm::mat4(1.f), m_CubePositions[i]) *
						glm::rotate(glm::mat4(1.f), glm::radians(m_CubeRotation), glm::vec3(0.f, 1.f, 0.f)) *
						// Move cube to (0, 0, 0), put this at last
						glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, -0.25f));
					m_DepthShader->SetUniformMat4f("u_Model", model_Cube);

					renderer.Draw(*m_CubeVAO, *m_CubeIBO, *m_DepthShader);
				}
			}
			glCullFace(GL_BACK);
			glDisable(GL_CULL_FACE);
		}

		// Second pass - render scene as normal with shadow mapping (using depth map)
		glViewport(0.f, 0.f, WINDOW_WIDTH, WINDOW_HEIGHT);
		// Back to default frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Render scene
		{
			m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
			m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);

			m_CubeShader->Bind();
			m_CubeShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);

			m_CubeShader->SetUniform3f("u_ViewPos", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);

			m_CubeShader->SetUniform1f("u_FarPlane", far);

			m_CubeDiffuseTexture->Bind();
			m_CubeSpecularTexture->Bind(1);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthCubemapBuffer);

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
			if (bMotionOn)
			{
				m_CubeRotation += m_ModelRotSpeed * deltaTime;
				m_CubeRotation = m_CubeRotation > 360.f ? 0.f : m_CubeRotation;
			}
			glEnable(GL_CULL_FACE);
			// Render cube
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
			glDisable(GL_CULL_FACE);
		}

	}

	void Test_OmniDirectionalShadowMapping::OnImGuiRender()
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

	void Test_OmniDirectionalShadowMapping::ProcessInput(GLFWwindow* window, float deltaTime)
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

	void Test_OmniDirectionalShadowMapping::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), s_FOVMin, s_FOVMax);
	}

	void Test_OmniDirectionalShadowMapping::ResetView()
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
