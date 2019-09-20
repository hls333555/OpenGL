#include "Test_DeferredShading.h"

#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>

#include "VertexBufferLayout.h"

// Put it at last!
#include <GLFW/glfw3.h>

#define DEFAULT_CAMERAFRONT glm::vec3(0.f, 0.f, -1.f)
#define DEFAULT_CAMERAPOS	DEFAULT_CAMERAFRONT * -m_CameraOrbitRadius
#define DEFAULT_YAW			-90.f
#define DEFAULT_PITCH		0.f
#define DEFAULT_FOV			45.f

namespace test
{
	float Test_DeferredShading::s_FOV = DEFAULT_FOV;
	float Test_DeferredShading::s_FOVMin = 1.f;
	float Test_DeferredShading::s_FOVMax = 90.f;

	Test_DeferredShading::Test_DeferredShading()
		: m_ModelRotSpeed(90.f)
		, m_ModelScale(0.2f)
		, m_AmbientIntensity(glm::vec3(0.16f, 0.16f, 0.16f))
		, m_DiffuseIntensity(glm::vec3(0.19f, 0.19f, 0.19f))
		, m_SpecularIntensity(glm::vec3(0.13f, 0.13f, 0.13f))
		, m_CameraOrbitRadius(6.f)
		, m_CameraPos(DEFAULT_CAMERAPOS)
		, m_CameraFront(DEFAULT_CAMERAFRONT)
		, m_CameraUp(glm::vec3(0.f, 1.f, 0.f))
		, m_CameraMoveSpeed(3.f)
		, m_CameraRotSpeed(15.f)
		, m_Yaw(DEFAULT_YAW), m_Pitch(DEFAULT_PITCH)
	{
		// WORKAROUND: C style function pointer must take a static function if it is a member function!
		glfwSetScrollCallback(Test::s_Window, OnMouseScroll);

		m_PointLightPositions = {
			{ 0.f,  1.f,  3.f},
			{-3.f,  0.f,  3.f},
			{ 3.f,  0.f,  3.f},
			{ 3.f,  0.f, -3.f},
			{-3.f,  0.f, -3.f}
		};

		m_PointLightColors = {
			{10.f, 0.f, 0.f},
			{0.f, 5.f, 0.f},
			{0.f, 0.f, 15.f},
			{1.f, 1.f, 1.f},
			{0.5f, 0.5f, 0.5f}
		};

		float vertices[] = {
			// Positions        
			// ---Begin: Top---
			0.f,  0.5f,  0.5f,
			0.5f, 0.5f,  0.5f,
			0.5f, 0.5f,  0.f,
			0.f,  0.5f,  0.f,
			// ---Begin: Front--
			0.f,  0.f,   0.5f,
			0.5f, 0.f,   0.5f,
			0.5f, 0.5f,  0.5f,
			0.f,  0.5f,  0.5f,
			// ---Begin: Left---
			0.f,  0.f,   0.f,  -
			0.f,  0.f,   0.5f, -
			0.f,  0.5f,  0.5f, -
			0.f,  0.5f,  0.f,  -
			// ---Begin: Back---
			0.f,  0.f,   0.f,
			0.5f, 0.f,   0.f,
			0.5f, 0.5f,  0.f,
			0.f,  0.5f,  0.f,
			// ---Begin: Right---
			0.5f, 0.f,   0.5f,
			0.5f, 0.f,   0.f,
			0.5f, 0.5f,  0.f,
			0.5f, 0.5f,  0.5f,
			// ---Begin: Bottom---
			0.f,  0.f,   0.f,
			0.5f, 0.f,   0.f,
			0.5f, 0.f,   0.5f,
			0.f,  0.f,   0.5f,
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

		float quadVertices[] = {
			-1.f, -1.f, 0.f, 0.f, // 1
			 1.f, -1.f, 1.f, 0.f, // 2
			 1.f,  1.f, 1.f, 1.f, // 3
			-1.f,  1.f, 0.f, 1.f  // 4
		};

		unsigned int quadIndices[] = {
			0, 1, 3,
			3, 1, 2
		};

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		////////////////////////////////////////////////////////////
		// Model ///////////////////////////////////////////////////
		////////////////////////////////////////////////////////////

		m_GeometryPassShader.reset(new Shader("res/shaders/Model_GBuffer.shader"));

		m_LightingPassShader.reset(new Shader("res/shaders/Model_DeferredShading.shader"));
		m_LightingPassShader->Bind();
		m_LightingPassShader->SetUniform1i("u_GPosition", 0);
		m_LightingPassShader->SetUniform1i("u_GNormal", 1);
		m_LightingPassShader->SetUniform1i("u_GColorSpec", 2);

		////////////////////////////////////////////////////////////
		// Quad ////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////

		m_QuadVAO.reset(new VertexArray());
		m_QuadVBO.reset(new VertexBuffer(quadVertices, 16 * sizeof(float)));
		VertexBufferLayout QuadLayout;
		QuadLayout.Push<float>(2);
		QuadLayout.Push<float>(2);
		m_QuadVAO->AddBuffer(*m_QuadVBO, QuadLayout);
		m_QuadIBO.reset(new IndexBuffer(quadIndices, 6));

		////////////////////////////////////////////////////////////
		// PointLight //////////////////////////////////////////////
		////////////////////////////////////////////////////////////

		m_PointLightVAO.reset(new VertexArray());

		m_PointLightVBO.reset(new VertexBuffer(vertices, 72 * sizeof(float)));

		VertexBufferLayout layout_PointLight;
		layout_PointLight.Push<float>(3);
		m_PointLightVAO->AddBuffer(*m_PointLightVBO, layout_PointLight);

		m_PointLightIBO.reset(new IndexBuffer(indices, 36));

		m_PointLightShader.reset(new Shader("res/shaders/BasicColor.shader"));

		////////////////////////////////////////////////////////////
		// GBuffer /////////////////////////////////////////////////
		////////////////////////////////////////////////////////////

		glCreateFramebuffers(1, &m_GBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer);

		// Position color buffer
		glCreateTextures(GL_TEXTURE_2D, 1, &m_GPositionTexture);
		glTextureStorage2D(m_GPositionTexture, 1, GL_RGB16F, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
		glTextureSubImage2D(m_GPositionTexture, 0, 0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, GL_RGB, GL_FLOAT, nullptr);
		glTextureParameteri(m_GPositionTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_GPositionTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GPositionTexture, 0);

		// Normal color buffer
		glCreateTextures(GL_TEXTURE_2D, 1, &m_GNormalTexture);
		glTextureStorage2D(m_GNormalTexture, 1, GL_RGB16F, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
		glTextureSubImage2D(m_GNormalTexture, 0, 0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, GL_RGB, GL_FLOAT, nullptr);
		glTextureParameteri(m_GNormalTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_GNormalTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GNormalTexture, 0);

		// Color + specular color buffer
		glCreateTextures(GL_TEXTURE_2D, 1, &m_GColorSpecTexture);
		glTextureStorage2D(m_GColorSpecTexture, 1, GL_RGBA8, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
		glTextureSubImage2D(m_GColorSpecTexture, 0, 0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTextureParameteri(m_GColorSpecTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_GColorSpecTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GColorSpecTexture, 0);

		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);

		// Depth buffer
		glCreateRenderbuffers(1, &m_RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

	void Test_DeferredShading::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;

		////////////////////////////////////////////////////////////
		// Deferred Rendering //////////////////////////////////////
		////////////////////////////////////////////////////////////

		// 1. Geometry pass: render scene's geometry/color data into GBuffer
		// -----------------------------------------------------------------

		glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
		m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
		// Render model
		if (m_Model)
		{
			if (m_bMotionOn)
			{
				m_ModelMotionRotation += m_ModelRotSpeed * deltaTime;
				m_ModelMotionRotation = m_ModelMotionRotation > 360.f ? 0.f : m_ModelMotionRotation;
			}
			{
				m_GeometryPassShader->Bind();
				m_GeometryPassShader->SetUniformMat4f("u_View", m_View);
				m_GeometryPassShader->SetUniformMat4f("u_Projection", m_Proj);
				for (unsigned int i = 0; i < 9; ++i)
				{
					glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(-3.f + i % 3 * 3, -2.35f, -2.f + i / 3 * 2)) *
						// Motion rotation
						glm::rotate(glm::mat4(1.f), glm::radians(m_ModelMotionRotation), glm::vec3(0.f, 1.f, 0.f)) *
						glm::scale(glm::mat4(1.f), glm::vec3(m_ModelScale));
					m_GeometryPassShader->SetUniformMat4f("u_Model", model);
					m_Model->Draw(*m_GeometryPassShader);
				}
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. Lighting pass: calculate lighting by iterating over a screen-filled quad pixel-by-pixel using the GBuffer's content
		// TODO: Using light volumes to improve performance for large numbers of lights
		// ----------------------------------------------------------------------------------------------------------------------

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Render quad
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_GPositionTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_GNormalTexture);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, m_GColorSpecTexture);
			m_LightingPassShader->Bind();
			for (unsigned int i = 0; i < m_PointLightPositions.size(); ++i)
			{
				m_LightingPassShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].color", m_PointLightColors[i].x, m_PointLightColors[i].y, m_PointLightColors[i].z);
				m_LightingPassShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].position", m_PointLightPositions[i].x, m_PointLightPositions[i].y, m_PointLightPositions[i].z);
				m_LightingPassShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].ambientIntensity", m_AmbientIntensity.x, m_AmbientIntensity.y, m_AmbientIntensity.z);
				m_LightingPassShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].diffuseIntensity", m_DiffuseIntensity.x, m_DiffuseIntensity.y, m_DiffuseIntensity.z);
				m_LightingPassShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].specularIntensity", m_SpecularIntensity.x, m_SpecularIntensity.y, m_SpecularIntensity.z);
				m_LightingPassShader->SetUniform1f("u_PointLights[" + std::to_string(i) + "].q", 0.032f);
				m_LightingPassShader->SetUniform1f("u_PointLights[" + std::to_string(i) + "].l", 0.09f);
				m_LightingPassShader->SetUniform1f("u_PointLights[" + std::to_string(i) + "].c", 1.f);
			}
			m_LightingPassShader->SetUniform3f("u_ViewPos", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);

			renderer.Draw(*m_QuadVAO, *m_QuadIBO, *m_LightingPassShader);
		}

		////////////////////////////////////////////////////////////
		// Forward Rendering ///////////////////////////////////////
		////////////////////////////////////////////////////////////

		// 2.5. Copy content of geometry's depth buffer to default framebuffer's depth buffer
		// ---------------------------------------------------------------------------------

		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, 0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. Render pointlights ON TOP OF scene
		// -------------------------------------

		// Render PointLights
		if (m_bDrawPointLights)
		{
			m_PointLightShader->Bind();
			m_PointLightShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);

			for (unsigned int i = 0; i < m_PointLightPositions.size(); ++i)
			{
				glm::mat4 model_PointLight = glm::translate(glm::mat4(1.f), m_PointLightPositions[i]) *
					glm::scale(glm::mat4(1.f), glm::vec3(0.2f));
				m_PointLightShader->SetUniformMat4f("u_Model", model_PointLight);
				m_PointLightShader->SetUniform4f("u_Color", m_PointLightColors[i].x, m_PointLightColors[i].y, m_PointLightColors[i].z, 1.f);

				renderer.Draw(*m_PointLightVAO, *m_PointLightIBO, *m_PointLightShader);
			}

		}

	}

	void Test_DeferredShading::OnImGuiRender()
	{
		if (ImGui::CollapsingHeader(u8"相机"))
		{
			ImGui::Text(u8"使用RMB，ALT+RMB，MMB和WSAD来变换相机视角！");
			ImGui::Text(u8"相机位置: (%.1f, %.1f, %.1f)", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
			ImGui::Text(u8"相机朝向: (%.1f, %.1f, %.1f)", m_CameraFront.x, m_CameraFront.y, m_CameraFront.z);

			if (ImGui::Button(u8"重置视角"))
			{
				ResetView();
			}
		}
		static bool bInitialLoad = true;
		static std::string modelPath("res/meshes/YYB Kagamine Rin_10th/YYB Kagamine Rin_10th_v1.0.pmx");
		if (bInitialLoad)
		{
			bInitialLoad = false;
			m_Model.reset(new Model(UTF8ToDefault(modelPath)));
		}
		if (ImGui::CollapsingHeader(u8"模型"))
		{
			////////////////////////////////////////////////////////////
			// Model Loading ///////////////////////////////////////////
			////////////////////////////////////////////////////////////

			ImGui::InputText(u8"模型路径", &modelPath);
			if (ImGui::Button(u8"导入模型"))
			{
				m_Model.reset(new Model(UTF8ToDefault(modelPath)));
			}

			ImGui::Checkbox(u8"观赏模式", &m_bMotionOn);
			ImGui::SliderFloat(u8"运动速度", &m_ModelRotSpeed, 10.f, 360.f);

			ImGui::InputFloat(u8"模型缩放", &m_ModelScale, 0.01f, 1.0f, "%.2f");
		}

		if (ImGui::CollapsingHeader(u8"光照"))
		{
			////////////////////////////////////////////////////////////
			// Lighting Controls ///////////////////////////////////////
			////////////////////////////////////////////////////////////

			ImGui::SliderFloat(u8"环境光强度", &m_AmbientIntensity.x, 0.f, 1.f);
			m_AmbientIntensity.z = m_AmbientIntensity.y = m_AmbientIntensity.x;

			ImGui::SliderFloat(u8"漫反射强度", &m_DiffuseIntensity.x, 0.f, 1.f);
			m_DiffuseIntensity.z = m_DiffuseIntensity.y = m_DiffuseIntensity.x;

			ImGui::SliderFloat(u8"高光强度", &m_SpecularIntensity.x, 0.f, 1.f);
			m_SpecularIntensity.z = m_SpecularIntensity.y = m_SpecularIntensity.x;

			ImGui::Checkbox(u8"使点光源可见", &m_bDrawPointLights);
		}

		//static bool bOpen = true;
		//ImGui::ShowDemoWindow(&bOpen);

	}

	void Test_DeferredShading::ProcessInput(GLFWwindow* window, float deltaTime)
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

	void Test_DeferredShading::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), s_FOVMin, s_FOVMax);
	}

	void Test_DeferredShading::ResetView()
	{
		m_bMotionOn = false;
		m_ModelMotionRotation = 0.f;

		m_CameraPos = DEFAULT_CAMERAPOS;
		m_CameraFront = DEFAULT_CAMERAFRONT;
		m_Yaw = DEFAULT_YAW;
		m_Pitch = DEFAULT_PITCH;
		s_FOV = DEFAULT_FOV;
	}

}
