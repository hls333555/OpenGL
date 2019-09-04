#include "Test_Model.h"

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
	float Test_Model::s_FOV = DEFAULT_FOV;
	float Test_Model::s_FOVMin = 1.f;
	float Test_Model::s_FOVMax = 90.f;

	Test_Model::Test_Model()
		: m_ModelRotSpeed(90.f)
		, m_ModelScale(0.2f)
		, m_Shininess(128.f)
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

		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it is closer to the camera than the former one
		glDepthFunc(GL_LESS);

		////////////////////////////////////////////////////////////
		// Model ///////////////////////////////////////////////////
		////////////////////////////////////////////////////////////

		m_ModelShader.reset(new Shader("res/shaders/Model.shader"));
		m_ModelNormalShader.reset(new Shader("res/shaders/ModelNormal.shader"));

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
		m_PointLightShader->Bind();
		m_PointLightShader->SetUniform4f("u_Color", 1.f, 1.f, 1.f, 1.f);

	}

	void Test_Model::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;
		m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
		m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
		if (m_Model)
		{
			if (m_bMotionOn)
			{
				m_ModelMotionRotation += m_ModelRotSpeed * deltaTime;
				m_ModelMotionRotation = m_ModelMotionRotation > 360.f ? 0.f : m_ModelMotionRotation;
			}
			glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -2.35f, 0.f)) *
				// Motion rotation
				glm::rotate(glm::mat4(1.f), glm::radians(m_ModelMotionRotation), glm::vec3(0.f, 1.f, 0.f)) *
				glm::scale(glm::mat4(1.f), glm::vec3(m_ModelScale));
			// Render model
			{
				m_ModelShader->Bind();
				m_ModelShader->SetUniformMat4f("u_Model", model);
				m_ModelShader->SetUniformMat4f("u_View", m_View);
				m_ModelShader->SetUniformMat4f("u_Projection", m_Proj);
				m_ModelShader->SetUniform3f("u_ViewPos", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);

				m_ModelShader->SetUniform1f("u_Material.shininess", m_Shininess);

				m_ModelShader->SetUniform3f("u_PointLights[0].position", m_PointLightPositions[0].x, m_PointLightPositions[0].y, m_PointLightPositions[0].z);
				m_ModelShader->SetUniform3f("u_PointLights[0].ambientIntensity", m_AmbientIntensity.x, m_AmbientIntensity.y, m_AmbientIntensity.z);
				m_ModelShader->SetUniform3f("u_PointLights[0].diffuseIntensity", m_DiffuseIntensity.x, m_DiffuseIntensity.y, m_DiffuseIntensity.z);
				m_ModelShader->SetUniform3f("u_PointLights[0].specularIntensity", m_SpecularIntensity.x, m_SpecularIntensity.y, m_SpecularIntensity.z);
				m_ModelShader->SetUniform1f("u_PointLights[0].q", 0.032f);
				m_ModelShader->SetUniform1f("u_PointLights[0].l", 0.09f);
				m_ModelShader->SetUniform1f("u_PointLights[0].c", 1.f);

				m_ModelShader->SetUniform3f("u_PointLights[1].position", m_PointLightPositions[1].x, m_PointLightPositions[1].y, m_PointLightPositions[1].z);
				m_ModelShader->SetUniform3f("u_PointLights[1].ambientIntensity", m_AmbientIntensity.x, m_AmbientIntensity.y, m_AmbientIntensity.z);
				m_ModelShader->SetUniform3f("u_PointLights[1].diffuseIntensity", m_DiffuseIntensity.x, m_DiffuseIntensity.y, m_DiffuseIntensity.z);
				m_ModelShader->SetUniform3f("u_PointLights[1].specularIntensity", m_SpecularIntensity.x, m_SpecularIntensity.y, m_SpecularIntensity.z);
				m_ModelShader->SetUniform1f("u_PointLights[1].q", 0.032f);
				m_ModelShader->SetUniform1f("u_PointLights[1].l", 0.09f);
				m_ModelShader->SetUniform1f("u_PointLights[1].c", 1.f);

				m_ModelShader->SetUniform3f("u_PointLights[2].position", m_PointLightPositions[2].x, m_PointLightPositions[2].y, m_PointLightPositions[2].z);
				m_ModelShader->SetUniform3f("u_PointLights[2].ambientIntensity", m_AmbientIntensity.x, m_AmbientIntensity.y, m_AmbientIntensity.z);
				m_ModelShader->SetUniform3f("u_PointLights[2].diffuseIntensity", m_DiffuseIntensity.x, m_DiffuseIntensity.y, m_DiffuseIntensity.z);
				m_ModelShader->SetUniform3f("u_PointLights[2].specularIntensity", m_SpecularIntensity.x, m_SpecularIntensity.y, m_SpecularIntensity.z);
				m_ModelShader->SetUniform1f("u_PointLights[2].q", 0.032f);
				m_ModelShader->SetUniform1f("u_PointLights[2].l", 0.09f);
				m_ModelShader->SetUniform1f("u_PointLights[2].c", 1.f);

				m_ModelShader->SetUniform3f("u_PointLights[3].position", m_PointLightPositions[3].x, m_PointLightPositions[3].y, m_PointLightPositions[3].z);
				m_ModelShader->SetUniform3f("u_PointLights[3].ambientIntensity", m_AmbientIntensity.x, m_AmbientIntensity.y, m_AmbientIntensity.z);
				m_ModelShader->SetUniform3f("u_PointLights[3].diffuseIntensity", m_DiffuseIntensity.x, m_DiffuseIntensity.y, m_DiffuseIntensity.z);
				m_ModelShader->SetUniform3f("u_PointLights[3].specularIntensity", m_SpecularIntensity.x, m_SpecularIntensity.y, m_SpecularIntensity.z);
				m_ModelShader->SetUniform1f("u_PointLights[3].q", 0.032f);
				m_ModelShader->SetUniform1f("u_PointLights[3].l", 0.09f);
				m_ModelShader->SetUniform1f("u_PointLights[3].c", 1.f);

				m_ModelShader->SetUniform3f("u_PointLights[4].position", m_PointLightPositions[4].x, m_PointLightPositions[4].y, m_PointLightPositions[4].z);
				m_ModelShader->SetUniform3f("u_PointLights[4].ambientIntensity", m_AmbientIntensity.x, m_AmbientIntensity.y, m_AmbientIntensity.z);
				m_ModelShader->SetUniform3f("u_PointLights[4].diffuseIntensity", m_DiffuseIntensity.x, m_DiffuseIntensity.y, m_DiffuseIntensity.z);
				m_ModelShader->SetUniform3f("u_PointLights[4].specularIntensity", m_SpecularIntensity.x, m_SpecularIntensity.y, m_SpecularIntensity.z);
				m_ModelShader->SetUniform1f("u_PointLights[4].q", 0.032f);
				m_ModelShader->SetUniform1f("u_PointLights[4].l", 0.09f);
				m_ModelShader->SetUniform1f("u_PointLights[4].c", 1.f);

				m_Model->Draw(*m_ModelShader);
			}
			// Render model normals
			if (m_bRenderModelNormals)
			{
				m_ModelNormalShader->Bind();
				m_ModelNormalShader->SetUniformMat4f("u_Model", model);
				m_ModelNormalShader->SetUniformMat4f("u_View", m_View);
				m_ModelNormalShader->SetUniformMat4f("u_Projection", m_Proj);

				m_Model->Draw(*m_ModelNormalShader);
			}
			
		}
		// Render PointLight
		if(m_bDrawPointLights)
		{
			m_PointLightShader->Bind();
			m_PointLightShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);

			for (unsigned int i = 0; i < m_PointLightPositions.size(); ++i)
			{
				glm::mat4 model_PointLight = glm::translate(glm::mat4(1.f), m_PointLightPositions[i]) *
					glm::scale(glm::mat4(1.f), glm::vec3(0.2f));
				m_PointLightShader->SetUniformMat4f("u_Model", model_PointLight);

				renderer.Draw(*m_PointLightVAO, *m_PointLightIBO, *m_PointLightShader);
			}

		}

	}

	void Test_Model::OnImGuiRender()
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
		
		if (ImGui::CollapsingHeader(u8"模型"))
		{
			////////////////////////////////////////////////////////////
			// Model Loading ///////////////////////////////////////////
			////////////////////////////////////////////////////////////

			static std::string modelPath("res/meshes/");
			ImGui::InputText(u8"模型路径", &modelPath);
			if (ImGui::Button(u8"导入模型"))
			{
				m_Model.reset(new Model(UTF8ToDefault(modelPath)));
				
				auto& meshes = m_Model->GetMeshes();
				m_DiffusePaths.clear();
				m_SpecularPaths.clear();
				m_DiffusePaths.resize(meshes.size(), "");
				m_SpecularPaths.resize(meshes.size(), "");
				// Set loaded texture paths as default
				for (unsigned int i = 0; i < meshes.size(); ++i)
				{
					auto& textures = meshes[i]->GetTextures();
					for (const auto& texture : textures)
					{
						auto str = DefaultToUTF8(texture->GetFilePath());
						switch (texture->GetType())
						{
						case TextureType::Diffuse:
							m_DiffusePaths[i] = str;
							break;
						case TextureType::Specular:
							m_SpecularPaths[i] = str;
							break;
						default:
							ASSERT(false);
							break;
						}
					}
				}
			}

			if (m_Model)
			{
				if (ImGui::CollapsingHeader(u8"贴图"))
				{
					////////////////////////////////////////////////////////////
					// Textures Loading ////////////////////////////////////////
					////////////////////////////////////////////////////////////

					auto& meshes = m_Model->GetMeshes();
					for (unsigned int i = 0; i < meshes.size(); ++i)
					{
						std::string diffText(u8"漫反射贴图：");
						ImGui::InputText((diffText + meshes[i]->GetName() + std::to_string(i)).c_str(), &m_DiffusePaths[i]);
						std::string diffButton(u8"导入漫反射贴图: ");
						if (ImGui::Button((diffButton + meshes[i]->GetName() + std::to_string(i)).c_str()))
						{
							meshes[i]->SetTexture(UTF8ToDefault(m_DiffusePaths[i]), TextureType::Diffuse);
						}

						std::string specText(u8"高光贴图：");
						ImGui::InputText((specText + meshes[i]->GetName() + std::to_string(i)).c_str(), &m_SpecularPaths[i]);
						std::string specButton(u8"导入高光贴图：");
						if (ImGui::Button((specButton + meshes[i]->GetName() + std::to_string(i)).c_str()))
						{
							meshes[i]->SetTexture(UTF8ToDefault(m_SpecularPaths[i]), TextureType::Specular);
						}
					}
				}
			}

			ImGui::Checkbox(u8"观赏模式", &m_bMotionOn);
			ImGui::SliderFloat(u8"运动速度", &m_ModelRotSpeed, 10.f, 360.f);

			ImGui::InputFloat(u8"模型缩放", &m_ModelScale, 0.01f, 1.0f, "%.2f");
			ImGui::Checkbox(u8"绘制法线", &m_bRenderModelNormals);
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

			ImGui::SliderFloat(u8"光泽度", &m_Shininess, 16.f, 256.f);

			ImGui::Checkbox(u8"使点光源可见", &m_bDrawPointLights);
		}

		//static bool bOpen = true;
		//ImGui::ShowDemoWindow(&bOpen);

	}

	void Test_Model::ProcessInput(GLFWwindow* window, float deltaTime)
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

	void Test_Model::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), s_FOVMin, s_FOVMax);
	}

	void Test_Model::ResetView()
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
