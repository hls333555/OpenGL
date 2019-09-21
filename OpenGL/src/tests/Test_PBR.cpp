#include "Test_PBR.h"

#include "VertexBufferLayout.h"

#include "imgui/imgui.h"
#include "glm/gtc/matrix_transform.hpp"
#include "stb_image/stb_image.h"

#include <iostream>

// Put it at last!
#include <GLFW/glfw3.h>

#define DEFAULT_CAMERAFRONT glm::vec3(0.f, 0.f, -1.f)
#define DEFAULT_CAMERAPOS	DEFAULT_CAMERAFRONT * -m_CameraOrbitRadius
#define DEFAULT_YAW			-90.f
#define DEFAULT_PITCH		0.f
#define DEFAULT_FOV			45.f

// Renders (and builds at first invocation) a sphere
// -------------------------------------------------
unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
	if (sphereVAO == 0)
	{
		glGenVertexArrays(1, &sphereVAO);

		unsigned int vbo, ebo;
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uv;
		std::vector<unsigned int> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359;
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back(glm::vec3(xPos, yPos, zPos));
				normals.push_back(glm::vec3(xPos, yPos, zPos));
				uv.push_back(glm::vec2(xSegment, ySegment));
			}
		}

		bool oddRow = false;
		for (int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		indexCount = indices.size();

		std::vector<float> data;
		for (int i = 0; i < positions.size(); ++i)
		{
			data.push_back(positions[i].x);
			data.push_back(positions[i].y);
			data.push_back(positions[i].z);
			if (normals.size() > 0)
			{
				data.push_back(normals[i].x);
				data.push_back(normals[i].y);
				data.push_back(normals[i].z);
			}
			if (uv.size() > 0)
			{
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
		}
		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		float stride = (3 + 3 + 2) * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	}

	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

namespace test
{
	float Test_BasicPBR::s_FOV = DEFAULT_FOV;
	float Test_BasicPBR::s_FOVMin = 1.f;
	float Test_BasicPBR::s_FOVMax = 90.f;

	Test_BasicPBR::Test_BasicPBR()
		: m_CameraOrbitRadius(3.f)
		, m_CameraPos(DEFAULT_CAMERAPOS)
		, m_CameraFront(DEFAULT_CAMERAFRONT)
		, m_CameraUp(glm::vec3(0.f, 1.f, 0.f))
		, m_CameraMoveSpeed(3.f)
		, m_CameraRotSpeed(15.f)
		, m_Yaw(DEFAULT_YAW), m_Pitch(DEFAULT_PITCH)
	{
		// WORKAROUND: C style function pointer must take a static function if it is a member function!
		glfwSetScrollCallback(Test::s_Window, OnMouseScroll);

		float vertices[] = {
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

		unsigned int indices[] = {
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

		m_PointLightPositions1 = {
			{ -10.f, -10.f, 10.f},
			{  10.f, -10.f, 10.f},
			{  10.f,  10.f, 10.f},
			{ -10.f,  10.f, 10.f}
		};

		m_PointLightPositions2 = {
			{ 0.f, 0.f, 10.f}
		};

		m_PointLightColors1 = {
			{300.f, 300.f, 300.f},
			{300.f, 300.f, 300.f},
			{300.f, 300.f, 300.f},
			{300.f, 300.f, 300.f},
		};

		m_PointLightColors2 = {
			{150.f, 150.f, 150.f}
		};

		glEnable(GL_DEPTH_TEST);

		m_VAO.reset(new VertexArray());

		m_VBO.reset(new VertexBuffer(vertices, 192 * sizeof(float)));

		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		layout.Push<float>(2);
		m_VAO->AddBuffer(*m_VBO, layout);

		m_IBO.reset(new IndexBuffer(indices, 36));

		m_Shader.reset(new Shader("res/shaders/BasicPBR.shader"));
		m_Shader->Bind();
		m_Shader->SetUniform3f("u_Material.baseColor", 0.5f, 0.f, 0.f);
		m_Shader->SetUniform1f("u_Material.ao", 1.f);

		// The texture may be loaded upside-down!
		m_BaseColorTexture.reset(new Texture("res/textures/PBR/rusted_iron/rustediron2_basecolor.png"));
		//m_NormalTexture.reset(new Texture("res/textures/PBR/rusted_iron/rustediron2_normal.png"));
		m_MetallicTexture.reset(new Texture("res/textures/PBR/rusted_iron/rustediron2_metallic.png"));
		m_RoughnessTexture.reset(new Texture("res/textures/PBR/rusted_iron/rustediron2_roughness.png"));

		m_Shader->SetUniform1i("u_Material2.baseColorMap", 0);
		//m_Shader->SetUniform1i("u_Material2.normalMap", 1);
		m_Shader->SetUniform1i("u_Material2.metallicMap", 2);
		m_Shader->SetUniform1i("u_Material2.roughnessMap", 3);
		m_Shader->SetUniform1f("u_Material2.ao", 1.f);

		m_PointLightShader.reset(new Shader("res/shaders/BasicColor.shader"));

	}

	void Test_BasicPBR::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;
		m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
		m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
		// Render spheres with varying metallic/roughness values scaled by rows and columns respectively
		{
			if (m_bUseTexture)
			{
				m_BaseColorTexture->Bind();
				//m_NormalTexture->Bind(1);
				m_MetallicTexture->Bind(2);
				m_RoughnessTexture->Bind(3);
			}
			
			m_PointLightPositions = m_bUseTexture ? &m_PointLightPositions2 : &m_PointLightPositions1;
			m_PointLightColors = m_bUseTexture ? &m_PointLightColors2 : &m_PointLightColors1;
			m_Shader->Bind();
			m_Shader->SetUniform1i("u_bUseTexture", m_bUseTexture);
			m_Shader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
			m_Shader->SetUniform3f("u_ViewPos", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
			for (unsigned int row = 0; row < 7; ++row)
			{
				m_Shader->SetUniform1f("u_Material.metallic", row / 6.f);
				for (unsigned int col = 0; col < 7; ++col)
				{
					m_Shader->SetUniform1f("u_Material.roughness", glm::clamp(col / 6.f, 0.05f, 1.f));

					glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3((col - 3.f) * 2.5f, (row - 3.f) * 2.5f, 0.f));
					m_Shader->SetUniformMat4f("u_Model", model);

					//renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
					renderSphere();
				}
			}
		}
		// Render PointLights
		if (true)
		{
			for (unsigned int i = 0; i < m_PointLightPositions->size(); ++i)
			{
				glm::vec3 newPos = (*m_PointLightPositions)[i] + glm::vec3(sin(glfwGetTime() * 5.f) * 5.f, 0.f, 0.f);
				newPos = (*m_PointLightPositions)[i];
				m_Shader->Bind();
				m_Shader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].position", newPos.x, newPos.y, newPos.z);
				m_Shader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].color", (*m_PointLightColors)[i].x, (*m_PointLightColors)[i].y, (*m_PointLightColors)[i].z);
				//m_Shader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].ambientIntensity", m_AmbientIntensity.x, m_AmbientIntensity.y, m_AmbientIntensity.z);
				//m_Shader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].diffuseIntensity", m_DiffuseIntensity.x, m_DiffuseIntensity.y, m_DiffuseIntensity.z);
				//m_Shader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].specularIntensity", m_SpecularIntensity.x, m_SpecularIntensity.y, m_SpecularIntensity.z);
				//m_Shader->SetUniform1f("u_PointLights[" + std::to_string(i) + "].q", 0.032f);
				//m_Shader->SetUniform1f("u_PointLights[" + std::to_string(i) + "].l", 0.09f);
				//m_Shader->SetUniform1f("u_PointLights[" + std::to_string(i) + "].c", 1.f);

				m_PointLightShader->Bind();
				m_PointLightShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
				glm::mat4 model_PointLight = glm::translate(glm::mat4(1.f), (*m_PointLightPositions)[i]) *
					glm::scale(glm::mat4(1.f), glm::vec3(0.2f));
				m_PointLightShader->SetUniformMat4f("u_Model", model_PointLight);
				m_PointLightShader->SetUniform4f("u_Color", (*m_PointLightColors)[i].x, (*m_PointLightColors)[i].y, (*m_PointLightColors)[i].z, 1.f);

				renderer.Draw(*m_VAO, *m_IBO, *m_PointLightShader);
			}

		}

	}

	void Test_BasicPBR::OnImGuiRender()
	{
		ImGui::Text(u8"使用RMB，ALT+RMB，MMB和WSAD来变换相机视角！");
		ImGui::Text(u8"相机位置: (%.1f, %.1f, %.1f)", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
		ImGui::Text(u8"相机朝向: (%.1f, %.1f, %.1f)", m_CameraFront.x, m_CameraFront.y, m_CameraFront.z);

		if (ImGui::Button(u8"重置视角"))
		{
			ResetView();
		}
		
		ImGui::Checkbox(u8"使用材质", &m_bUseTexture);

	}

	void Test_BasicPBR::ProcessInput(GLFWwindow* window, float deltaTime)
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

	void Test_BasicPBR::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), s_FOVMin, s_FOVMax);
	}

	void Test_BasicPBR::ResetView()
	{
		m_CameraPos = DEFAULT_CAMERAPOS;
		m_CameraFront = DEFAULT_CAMERAFRONT;
		m_Yaw = DEFAULT_YAW;
		m_Pitch = DEFAULT_PITCH;
		s_FOV = DEFAULT_FOV;
	}

}
