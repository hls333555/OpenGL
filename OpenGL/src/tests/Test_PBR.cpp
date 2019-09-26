#include "Test_PBR.h"

#include "VertexBufferLayout.h"

#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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
		const float PI = 3.14159265359f;
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.f * PI) * std::sin(ySegment * PI);

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
		indexCount = (unsigned int)indices.size();

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
		int stride = (3 + 3 + 2) * sizeof(float);
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
		, m_CameraMoveSpeed(10.f)
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
		glDisable(GL_CULL_FACE);

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

		m_BaseColorTexture.reset(new Texture("res/textures/PBR/bamboo_wood/color.png"));
		m_NormalTexture.reset(new Texture("res/textures/PBR/bamboo_wood/normal.png"));
		m_MetallicTexture.reset(new Texture("res/textures/PBR/bamboo_wood/metal.png"));
		m_RoughnessTexture.reset(new Texture("res/textures/PBR/bamboo_wood/roughness.png"));
		m_AOTexture.reset(new Texture("res/textures/PBR/bamboo_wood/ao.png"));

		m_Shader->SetUniform1i("u_Material.baseColorMap", 0);
		m_Shader->SetUniform1i("u_Material.normalMap", 1);
		m_Shader->SetUniform1i("u_Material.metallicMap", 2);
		m_Shader->SetUniform1i("u_Material.roughnessMap", 3);
		m_Shader->SetUniform1i("u_Material.aoMap", 4);

		m_PointLightShader.reset(new Shader("res/shaders/BasicColor.shader"));

	}

	void Test_BasicPBR::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;
		m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
		m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
		// Render spheres with textures or with varying metallic/roughness values scaled by rows and columns respectively
		{
			if (m_bUseTextures)
			{
				m_BaseColorTexture->Bind();
				m_NormalTexture->Bind(1);
				m_MetallicTexture->Bind(2);
				m_RoughnessTexture->Bind(3);
				m_AOTexture->Bind(4);
				m_PointLightPositions = &m_PointLightPositions2;
				m_PointLightColors = &m_PointLightColors2;
			}
			else
			{
				m_PointLightPositions = &m_PointLightPositions1;
				m_PointLightColors = &m_PointLightColors1;
			}
			
			m_Shader->Bind();
			m_Shader->SetUniform1i("u_bUseTextures", m_bUseTextures);
			m_Shader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
			m_Shader->SetUniform3f("u_ViewPos", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
			for (unsigned int row = 0; row < 7; ++row)
			{
				m_Shader->SetUniform1f("u_Material.metallic", row / 7.f);
				for (unsigned int col = 0; col < 7; ++col)
				{
					// We clamp the roughness to 0.05f - 1.f
					// as perfectly smooth surfaces (roughness of 0.f) tend to look a bit off on direct lighting
					m_Shader->SetUniform1f("u_Material.roughness", glm::clamp(col / 7.f, 0.05f, 1.f));

					glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3((col - 3.f) * 2.5f, (row - 3.f) * 2.5f, 0.f));
					m_Shader->SetUniformMat4f("u_Model", model);

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
		
		ImGui::Checkbox(u8"使用材质", &m_bUseTextures);

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


	float Test_IBLPBR::s_FOV = DEFAULT_FOV;
	float Test_IBLPBR::s_FOVMin = 1.f;
	float Test_IBLPBR::s_FOVMax = 90.f;

	Test_IBLPBR::Test_IBLPBR()
		: m_ModelRotSpeed(90.f)
		, m_ModelScale(0.05f)
		, m_CameraOrbitRadius(6.f)
		, m_CameraPos(DEFAULT_CAMERAPOS)
		, m_CameraFront(DEFAULT_CAMERAFRONT)
		, m_CameraUp(glm::vec3(0.f, 1.f, 0.f))
		, m_CameraMoveSpeed(10.f)
		, m_CameraRotSpeed(15.f)
		, m_Yaw(DEFAULT_YAW), m_Pitch(DEFAULT_PITCH)
		, m_BackgroundStyle(2)
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

		float backgroundVertices[] = {
			// ---Begin: Top---
			-10.f,  10.f, -10.f, // 0
			 10.f,  10.f, -10.f, // 1
			 10.f,  10.f,  10.f, // 2
			-10.f,  10.f,  10.f, // 3
			// ---Begin: Front---
			 10.f, -10.f,  10.f, // 4
			-10.f, -10.f,  10.f, // 5
			-10.f,  10.f,  10.f, // 6
			 10.f,  10.f,  10.f, // 7
			// ---Begin: Left---
			-10.f, -10.f,  10.f, // 8
			-10.f, -10.f, -10.f, // 9
			-10.f,  10.f, -10.f, // 10
			-10.f,  10.f,  10.f, // 11
			// ---Begin: Back---
			-10.f, -10.f, -10.f, // 12
			 10.f, -10.f, -10.f, // 13
			 10.f,  10.f, -10.f, // 14
			-10.f,  10.f, -10.f, // 15
			// ---Begin: Right---
			 10.f, -10.f, -10.f, // 16 
			 10.f, -10.f,  10.f, // 17
			 10.f,  10.f,  10.f, // 18
			 10.f,  10.f, -10.f, // 19
			// ---Begin: Bottom---
			-10.f, -10.f,  10.f, // 20
			 10.f, -10.f,  10.f, // 21
			 10.f, -10.f, -10.f, // 22
			-10.f, -10.f, -10.f  // 23
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

		m_PointLightPositions1 = {
			{ -10.f, -10.f, 10.f},
			{  10.f, -10.f, 10.f},
			{  10.f,  10.f, 10.f},
			{ -10.f,  10.f, 10.f}
		};

		m_PointLightColors1 = {
			{300.f, 300.f, 300.f},
			{300.f, 300.f, 300.f},
			{300.f, 300.f, 300.f},
			{300.f, 300.f, 300.f},
		};

		m_PointLightPositions2 = {
			{ 0.f, 0.f, 3.f},
			{ 0.f, 0.f, -3.f}
		};

		m_PointLightColors2 = {
			{10.f, 10.f, 10.f},
			{10.f, 10.f, 10.f},
		};

		glEnable(GL_DEPTH_TEST);
		// Set depth function to less than AND equal for background depth trick
		glDepthFunc(GL_LEQUAL);
		glDisable(GL_CULL_FACE);
		// enable seamless cubemap sampling for lower mip levels in the pre-filter map.
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		m_VAO.reset(new VertexArray());

		m_VBO.reset(new VertexBuffer(vertices, 192 * sizeof(float)));

		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		layout.Push<float>(2);
		m_VAO->AddBuffer(*m_VBO, layout);

		m_IBO.reset(new IndexBuffer(indices, 36));

		m_PBRShader.reset(new Shader("res/shaders/IBLPBR.shader"));
		m_PBRShader->Bind();
		// Bindings are set in shader directly
		//m_PBRShader->SetUniform1i("u_IrradianceMap", 0);
		//m_PBRShader->SetUniform1i("u_PrefilterMap", 1);
		//m_PBRShader->SetUniform1i("u_BRDFLUT", 2);
		//m_PBRShader->SetUniform1i("baseColorMap", 3);
		//m_PBRShader->SetUniform1i("normalMap", 4);
		//m_PBRShader->SetUniform1i("metallicMap", 5);
		//m_PBRShader->SetUniform1i("roughnessMap", 6);
		//m_PBRShader->SetUniform1i("aoMap", 7);
		m_PBRShader->SetUniform3f("u_Material.baseColor", 1.f, 1.f, 1.f);
		m_PBRShader->SetUniform1f("u_Material.ao", 1.f);

		m_ModelDefaultShader.reset(new Shader("res/shaders/WhiteColorWithLighting.shader"));

		m_BackgroundVAO.reset(new VertexArray());
		m_BackgroundVBO.reset(new VertexBuffer(backgroundVertices, 72 * sizeof(float)));
		VertexBufferLayout backgroundLayout;
		backgroundLayout.Push<float>(3);
		m_BackgroundVAO->AddBuffer(*m_BackgroundVBO, backgroundLayout);

		m_BackgroundShader.reset(new Shader("res/shaders/PBRBackground.shader"));
		m_BackgroundShader->Bind();
		m_BackgroundShader->SetUniform1i("u_EnvironmentMap", 0);

		m_PointLightShader.reset(new Shader("res/shaders/BasicColor.shader"));

		m_QuadVAO.reset(new VertexArray());
		m_QuadVBO.reset(new VertexBuffer(quadVertices, 16 * sizeof(float)));
		VertexBufferLayout quadLayout;
		quadLayout.Push<float>(2);
		quadLayout.Push<float>(2);
		m_QuadVAO->AddBuffer(*m_QuadVBO, quadLayout);
		m_QuadIBO.reset(new IndexBuffer(quadIndices, 6));

		// Setup framebuffer
		// -----------------

		glCreateFramebuffers(1, &m_CaptureFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);

		glCreateRenderbuffers(1, &m_CaptureRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_CaptureRBO);

		// Load the HDR environment map
		// ----------------------------

		stbi_set_flip_vertically_on_load(1);
		int width, height, numComponents;
		float* data = stbi_loadf("res/textures/HDR/Alexs_Apt_2k.hdr", &width, &height, &numComponents, 0);
		if (data)
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_HDRTexture);
			glBindTexture(GL_TEXTURE_2D, m_HDRTexture);
			glTextureStorage2D(m_HDRTexture, 1, GL_RGB16F, width, height);
			glTextureSubImage2D(m_HDRTexture, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, data);

			glTextureParameteri(m_HDRTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_HDRTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_HDRTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_HDRTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load HDR image!" << std::endl;
		}

		// Setup cubemap to render to and attach to framebuffer
		// ----------------------------------------------------

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_EnvCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTextureParameteri(m_EnvCubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_EnvCubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_EnvCubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_EnvCubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_EnvCubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// Set up projection and view matrices for capturing data onto the 6 cubemap face directions
		// -----------------------------------------------------------------------------------------

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] = {
			glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		// Convert HDR equirectangular environment map to cubemap equivalent
		// -----------------------------------------------------------------

		m_EquirectangularToCubemapShader.reset(new Shader("res/shaders/EquirectangularToCubemap.shader"));
		m_EquirectangularToCubemapShader->Bind();
		m_EquirectangularToCubemapShader->SetUniform1i("u_EquirectangularMap", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_HDRTexture);
		
		Renderer renderer;

		glViewport(0, 0, 512, 512); // Don't forget to configure the viewport to the capture dimensions
		glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			m_EquirectangularToCubemapShader->SetUniformMat4f("u_ViewProjection", captureProjection * captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_EnvCubemap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderer.Draw(*m_BackgroundVAO, *m_IBO, *m_EquirectangularToCubemapShader);
		}
		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Create an irradiance cubemap, and re-scale capture FBO to irradiance scale
		// --------------------------------------------------------------------------

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_IrradianceMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTextureParameteri(m_IrradianceMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_IrradianceMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_IrradianceMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_IrradianceMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_IrradianceMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

		// Solve diffuse integral by convolution to create an irradiance (cube)map
		// -----------------------------------------------------------------------

		m_IrradianceShader.reset(new Shader("res/shaders/IrradianceConvolution.shader"));
		m_IrradianceShader->Bind();
		m_IrradianceShader->SetUniform1i("u_EnvironmentMap", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);

		glViewport(0, 0, 32, 32); // Don't forget to configure the viewport to the capture dimensions
		glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			m_IrradianceShader->SetUniformMat4f("u_ViewProjection", captureProjection * captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_IrradianceMap, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderer.Draw(*m_BackgroundVAO, *m_IBO, *m_IrradianceShader);
		}
		ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale
		// -------------------------------------------------------------------------

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_PrefilterMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTextureParameteri(m_PrefilterMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Be sure to set minifcation filter to mip_linear
		glTextureParameteri(m_PrefilterMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_PrefilterMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_PrefilterMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_PrefilterMap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// Generate mipmaps for the cubemap so OpenGL automatically allocates the required memory
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		// Run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map
		// ----------------------------------------------------------------------------------------------
		
		m_PrefilterShader.reset(new Shader("res/shaders/Prefilter.shader"));
		m_PrefilterShader->Bind();
		m_PrefilterShader->SetUniform1i("u_EnvironmentMap", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);

		glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			// Reisze framebuffer according to mip-level size
			double mipWidth = 128 * std::pow(0.5f, mip);
			double mipHeight = 128 * std::pow(0.5f, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, (int)mipWidth, (int)mipHeight);
			glViewport(0, 0, (int)mipWidth, (int)mipHeight);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			m_PrefilterShader->SetUniform1f("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i)
			{
				m_PrefilterShader->SetUniformMat4f("u_ViewProjection", captureProjection* captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_PrefilterMap, mip);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				renderer.Draw(*m_BackgroundVAO, *m_IBO, *m_PrefilterShader);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Generate a 2D LUT from the BRDF equations used
		// ----------------------------------------------

		glCreateTextures(GL_TEXTURE_2D, 1, &m_BRDFLUTTexture);
		// Rre-allocate enough memory for the LUT texture
		glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTexture);
		glTextureStorage2D(m_BRDFLUTTexture, 1, GL_RG16F, 512, 512);
		glTextureSubImage2D(m_BRDFLUTTexture, 0, 0, 0, 512, 512, GL_RG, GL_FLOAT, nullptr);
		glTextureParameteri(m_BRDFLUTTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_BRDFLUTTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Be sure to set wrapping mode to GL_CLAMP_TO_EDGE
		glTextureParameteri(m_BRDFLUTTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_BRDFLUTTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
		glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BRDFLUTTexture, 0);

		glViewport(0, 0, 512, 512);
		m_BRDFShader.reset(new Shader("res/shaders/BRDF.shader"));
		m_BRDFShader->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		renderer.Draw(*m_QuadVAO, *m_QuadIBO, *m_BRDFShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// -----------------------------------------------------------------------

		// Reset the viewport to the original framebuffer's screen dimensions
		glViewport(0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT);

	}

	void Test_IBLPBR::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;
		m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f);
		m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
		// Bind pre-computed IBL data
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_BRDFLUTTexture);

		m_PBRShader->Bind();
		m_PBRShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
		m_PBRShader->SetUniform3f("u_ViewPos", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
		if (m_bUseModel)
		{
			if (m_Model)
			{
				if (m_bMotionOn)
				{
					m_ModelMotionRotation += m_ModelRotSpeed * deltaTime;
					m_ModelMotionRotation = m_ModelMotionRotation > 360.f ? 0.f : m_ModelMotionRotation;
				}
				m_PointLightPositions = &m_PointLightPositions2;
				m_PointLightColors = &m_PointLightColors2;
				// Render PBR model
				{
					glm::mat4 model =
						// Motion rotation
						glm::rotate(glm::mat4(1.f), glm::radians(m_ModelMotionRotation), glm::vec3(0.f, 1.f, 0.f)) *
						// Make Z point up to cater to user's habbits
						glm::translate(glm::mat4(1.f), glm::vec3(m_ModelTranslation.x, m_ModelTranslation.z, m_ModelTranslation.y)) *
						// TODO: change these to quaternions in the future
						glm::rotate(glm::mat4(1.f), glm::radians(m_ModelRotation.x), glm::vec3(1.f, 0.f, 0.f)) *
						glm::rotate(glm::mat4(1.f), glm::radians(m_ModelRotation.z), glm::vec3(0.f, 1.f, 0.f)) *
						glm::rotate(glm::mat4(1.f), glm::radians(m_ModelRotation.y), glm::vec3(0.f, 0.f, 1.f)) *
						glm::scale(glm::mat4(1.f), glm::vec3(m_ModelScale));

					m_ModelDefaultShader->Bind();
					m_ModelDefaultShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
					m_ModelDefaultShader->SetUniform3f("u_ViewPos", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
					m_ModelDefaultShader->SetUniformMat4f("u_Model", model);

					m_PBRShader->Bind();
					m_PBRShader->SetUniform1i("u_bUseTextures", true);
					m_PBRShader->SetUniformMat4f("u_Model", model);

					m_Model->Draw(*m_PBRShader, true, m_ModelDefaultShader);
				}
			}
		}
		// Render spheres
		else
		{
			m_bFirstUseModel = true;

			m_PointLightPositions = &m_PointLightPositions1;
			m_PointLightColors = &m_PointLightColors1;

			m_PBRShader->Bind();
			m_PBRShader->SetUniform1i("u_bUseTextures", m_bUseTextures);
			// Render spheres with textures
			if (m_bUseTextures)
			{
				if (m_BaseColorTextures.size() == 0)
				{
					m_BaseColorTextures.push_back(std::make_unique<Texture>("res/textures/PBR/bamboo_wood/color.png"));
					m_NormalTextures.push_back(std::make_unique<Texture>("res/textures/PBR/bamboo_wood/normal.png"));
					m_MetallicTextures.push_back(std::make_unique<Texture>("res/textures/PBR/bamboo_wood/metallic.png"));
					m_RoughnessTextures.push_back(std::make_unique<Texture>("res/textures/PBR/bamboo_wood/roughness.png"));
					m_AOTextures.push_back(std::make_unique<Texture>("res/textures/PBR/bamboo_wood/ao.png"));

					m_BaseColorTextures.push_back(std::make_unique<Texture>("res/textures/PBR/gold/color.png"));
					m_NormalTextures.push_back(std::make_unique<Texture>("res/textures/PBR/gold/normal.png"));
					m_MetallicTextures.push_back(std::make_unique<Texture>("res/textures/PBR/gold/metallic.png"));
					m_RoughnessTextures.push_back(std::make_unique<Texture>("res/textures/PBR/gold/roughness.png"));
					m_AOTextures.push_back(std::make_unique<Texture>("res/textures/PBR/gold/ao.png"));

					m_BaseColorTextures.push_back(std::make_unique<Texture>("res/textures/PBR/grass/color.png"));
					m_NormalTextures.push_back(std::make_unique<Texture>("res/textures/PBR/grass/normal.png"));
					m_MetallicTextures.push_back(std::make_unique<Texture>("res/textures/PBR/grass/metallic.png"));
					m_RoughnessTextures.push_back(std::make_unique<Texture>("res/textures/PBR/grass/roughness.png"));
					m_AOTextures.push_back(std::make_unique<Texture>("res/textures/PBR/grass/ao.png"));

					m_BaseColorTextures.push_back(std::make_unique<Texture>("res/textures/PBR/plastic/color.png"));
					m_NormalTextures.push_back(std::make_unique<Texture>("res/textures/PBR/plastic/normal.png"));
					m_MetallicTextures.push_back(std::make_unique<Texture>("res/textures/PBR/plastic/metallic.png"));
					m_RoughnessTextures.push_back(std::make_unique<Texture>("res/textures/PBR/plastic/roughness.png"));
					m_AOTextures.push_back(std::make_unique<Texture>("res/textures/PBR/plastic/ao.png"));

					m_BaseColorTextures.push_back(std::make_unique<Texture>("res/textures/PBR/rusted_iron/color.png"));
					m_NormalTextures.push_back(std::make_unique<Texture>("res/textures/PBR/rusted_iron/normal.png"));
					m_MetallicTextures.push_back(std::make_unique<Texture>("res/textures/PBR/rusted_iron/metallic.png"));
					m_RoughnessTextures.push_back(std::make_unique<Texture>("res/textures/PBR/rusted_iron/roughness.png"));
					m_AOTextures.push_back(std::make_unique<Texture>("res/textures/PBR/rusted_iron/ao.png"));

					m_BaseColorTextures.push_back(std::make_unique<Texture>("res/textures/PBR/wall/color.png"));
					m_NormalTextures.push_back(std::make_unique<Texture>("res/textures/PBR/wall/normal.png"));
					m_MetallicTextures.push_back(std::make_unique<Texture>("res/textures/PBR/wall/metallic.png"));
					m_RoughnessTextures.push_back(std::make_unique<Texture>("res/textures/PBR/wall/roughness.png"));
					m_AOTextures.push_back(std::make_unique<Texture>("res/textures/PBR/gold/ao.png"));
				}
				for (unsigned int i = 0; i < m_BaseColorTextures.size(); ++i)
				{
					m_BaseColorTextures[i]->Bind(3);
					m_NormalTextures[i]->Bind(4);
					m_MetallicTextures[i]->Bind(5);
					m_RoughnessTextures[i]->Bind(6);
					m_AOTextures[i]->Bind(7);

					glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(-4.f + i * 2, 0.f, 0.f));
					m_PBRShader->SetUniformMat4f("u_Model", model);
					renderSphere();
				}
			}
			// Render spheres with varying metallic/roughness values scaled by rows and columns respectively
			else
			{
				for (unsigned int row = 0; row < 7; ++row)
				{
					m_PBRShader->Bind();
					m_PBRShader->SetUniform1f("u_Material.metallic", row / 7.f);
					for (unsigned int col = 0; col < 7; ++col)
					{
						// We clamp the roughness to 0.05f - 1.f
						// as perfectly smooth surfaces (roughness of 0.f) tend to look a bit off on direct lighting
						m_PBRShader->SetUniform1f("u_Material.roughness", glm::clamp(col / 7.f, 0.05f, 1.f));

						glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3((col - 3.f) * 2.5f, (row - 3.f) * 2.5f, 0.f));
						m_PBRShader->SetUniformMat4f("u_Model", model);

						renderSphere();
					}
				}
			}
		}
		for (unsigned int i = 0; i < (*m_PointLightPositions).size(); ++i)
		{
			glm::vec3 newPos = (*m_PointLightPositions)[i] + glm::vec3(sin(glfwGetTime() * 5.f) * 5.f, 0.f, 0.f);
			newPos = (*m_PointLightPositions)[i];

			m_ModelDefaultShader->Bind();
			m_ModelDefaultShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].position", newPos.x, newPos.y, newPos.z);

			m_PBRShader->Bind();
			m_PBRShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].position", newPos.x, newPos.y, newPos.z);
			m_PBRShader->SetUniform3f("u_PointLights[" + std::to_string(i) + "].color", (*m_PointLightColors)[i].x, (*m_PointLightColors)[i].y, (*m_PointLightColors)[i].z);

			// Render pointLights
			if (m_bRenderPointlights)
			{
				m_PointLightShader->Bind();
				m_PointLightShader->SetUniformMat4f("u_ViewProjection", m_Proj* m_View);
				glm::mat4 model_PointLight = glm::translate(glm::mat4(1.f), (*m_PointLightPositions)[i]) *
					glm::scale(glm::mat4(1.f), glm::vec3(0.2f));
				m_PointLightShader->SetUniformMat4f("u_Model", model_PointLight);
				m_PointLightShader->SetUniform4f("u_Color", (*m_PointLightColors)[i].x, (*m_PointLightColors)[i].y, (*m_PointLightColors)[i].z, 1.f);

				renderer.Draw(*m_VAO, *m_IBO, *m_PointLightShader);
			}
			
		}
		// Render background cubemap at last to prevent overdraw
		{
			glActiveTexture(GL_TEXTURE0);
			switch (m_BackgroundStyle)
			{
			case 1:
				glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
				break;
			case 2:
				// Display irradiance map
				glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
				break;
			case 3:
				// Display prefilter map
				glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
				break;
			default:
				ASSERT(false);
				break;
			}
			
			m_BackgroundShader->Bind();
			m_BackgroundShader->SetUniformMat4f("u_View", m_View);
			m_BackgroundShader->SetUniformMat4f("u_Projection", m_Proj);

			renderer.Draw(*m_BackgroundVAO, *m_IBO, *m_BackgroundShader);
		}

		// Render BRDF map to screen
		//m_BRDFShader->Bind();
		//renderer.Draw(*m_QuadVAO, *m_QuadIBO, *m_BRDFShader);

	}

	void Test_IBLPBR::OnImGuiRender()
	{
		ImGui::Text(u8"使用RMB，ALT+RMB，MMB和WSAD来变换相机视角！");
		ImGui::Text(u8"相机位置: (%.1f, %.1f, %.1f)", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
		ImGui::Text(u8"相机朝向: (%.1f, %.1f, %.1f)", m_CameraFront.x, m_CameraFront.y, m_CameraFront.z);

		if (ImGui::Button(u8"重置视角"))
		{
			ResetView();
		}

		ImGui::SliderInt(u8"背景样式", &m_BackgroundStyle, 1, 3);

		ImGui::Checkbox(u8"绘制点光源", &m_bRenderPointlights);

		if (ImGui::Checkbox(u8"展示PBR材质", &m_bUseTextures))
		{
			m_bUseModel = false;
		}

		if (ImGui::Checkbox(u8"展示PBR模型", &m_bUseModel))
		{
			m_bUseTextures = false;
		}

		if (m_bUseModel)
		{
			////////////////////////////////////////////////////////////
			// Model Loading ///////////////////////////////////////////
			////////////////////////////////////////////////////////////

			if (m_bFirstUseModel || ImGui::CollapsingHeader(u8"模型"))
			{
				m_bFirstUseModel = false;

				static std::string modelPath("res/meshes/PBR/AK47/AK47.fbx");
				ImGui::InputText(u8"模型路径", &modelPath);
				ImGui::Text(u8"注意！导入非PBR模型可能会出现渲染错误！");
				if (!m_Model || ImGui::Button(u8"导入模型"))
				{
					if (!m_Model || m_Model->GetFullDir() != modelPath)
					{
						m_Model.reset(new Model(UTF8ToDefault(modelPath)));

						m_BaseColorPaths.clear();
						m_NormalPaths.clear();
						m_MetallicPaths.clear();
						m_RoughnessPaths.clear();
						m_AOPaths.clear();

						auto& meshes = m_Model->GetMeshes();
						m_BaseColorPaths.resize(meshes.size(), "");
						m_NormalPaths.resize(meshes.size(), "");
						m_MetallicPaths.resize(meshes.size(), "");
						m_RoughnessPaths.resize(meshes.size(), "");
						m_AOPaths.resize(meshes.size(), "");
						// Set loaded texture paths as default
						for (unsigned int i = 0; i < meshes.size(); ++i)
						{
							auto& textures = meshes[i]->GetTextures();
							for (const auto& texture : textures)
							{
								auto str = DefaultToUTF8(texture->GetFilePath());
								switch (texture->GetType())
								{
								case TextureType::BaseColor:
									m_BaseColorPaths[i] = str;
									break;
								case TextureType::Normal:
									m_NormalPaths[i] = str;
									break;
								case TextureType::Metallic:
									m_MetallicPaths[i] = str;
									break;
								case TextureType::Roughness:
									m_RoughnessPaths[i] = str;
									break;
								case TextureType::AO:
									m_AOPaths[i] = str;
									break;
								default:
									ASSERT(false);
									break;
								}
							}
						}

					}

				}

				ImGui::Checkbox(u8"观赏模式", &m_bMotionOn);
				ImGui::SliderFloat(u8"运动速度", &m_ModelRotSpeed, 10.f, 360.f);

				ImGui::InputFloat3(u8"平移", glm::value_ptr(m_ModelTranslation), "%.2f");
				ImGui::InputFloat3(u8"旋转", glm::value_ptr(m_ModelRotation), "%.2f");
				ImGui::InputFloat(u8"缩放", &m_ModelScale, 0.01f, 1.f, "%.2f");

				//ImGui::Checkbox(u8"绘制法线", &m_bRenderModelNormals);
			}

			////////////////////////////////////////////////////////////
			// Textures Loading ////////////////////////////////////////
			////////////////////////////////////////////////////////////

			if (m_Model && ImGui::CollapsingHeader(u8"贴图"))
			{
				auto& meshes = m_Model->GetMeshes();
				for (unsigned int i = 0; i < meshes.size(); ++i)
				{
					const std::string& meshName = meshes[i]->GetName();
					if (ImGui::CollapsingHeader((u8"网格：" + meshName).c_str()))
					{
						std::string baseColorText(u8"BaseColor贴图路径：");
						ImGui::InputText((baseColorText + std::to_string(i)).c_str(), &m_BaseColorPaths[i]);
						std::string baseColorButton(u8"导入BaseColor贴图: ");
						if (ImGui::Button((baseColorButton + std::to_string(i)).c_str()))
						{
							meshes[i]->SetTexture(UTF8ToDefault(m_BaseColorPaths[i]), TextureType::BaseColor);
						}

						std::string normalText(u8"Normal贴图路径：");
						ImGui::InputText((normalText + std::to_string(i)).c_str(), &m_NormalPaths[i]);
						std::string normalButton(u8"导入Normal贴图: ");
						if (ImGui::Button((normalButton + std::to_string(i)).c_str()))
						{
							meshes[i]->SetTexture(UTF8ToDefault(m_NormalPaths[i]), TextureType::Normal);
						}

						std::string metallicText(u8"Metallic贴图路径：");
						ImGui::InputText((metallicText + std::to_string(i)).c_str(), &m_MetallicPaths[i]);
						std::string metallicButton(u8"导入Metallic贴图: ");
						if (ImGui::Button((metallicButton + std::to_string(i)).c_str()))
						{
							meshes[i]->SetTexture(UTF8ToDefault(m_MetallicPaths[i]), TextureType::Metallic);
						}

						std::string roughnessText(u8"Roughness贴图路径：");
						ImGui::InputText((roughnessText + std::to_string(i)).c_str(), &m_RoughnessPaths[i]);
						std::string roughnessButton(u8"导入Roughness贴图: ");
						if (ImGui::Button((roughnessButton + std::to_string(i)).c_str()))
						{
							meshes[i]->SetTexture(UTF8ToDefault(m_RoughnessPaths[i]), TextureType::Roughness);
						}

						std::string aoText(u8"AO贴图路径：");
						ImGui::InputText((aoText + std::to_string(i)).c_str(), &m_AOPaths[i]);
						std::string aoButton(u8"导入AO贴图: ");
						if (ImGui::Button((aoButton + std::to_string(i)).c_str()))
						{
							meshes[i]->SetTexture(UTF8ToDefault(m_AOPaths[i]), TextureType::AO);
						}
					}
				}
			}
		}

	}

	void Test_IBLPBR::ProcessInput(GLFWwindow* window, float deltaTime)
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

	void Test_IBLPBR::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), s_FOVMin, s_FOVMax);
	}

	void Test_IBLPBR::ResetView()
	{
		m_CameraPos = DEFAULT_CAMERAPOS;
		m_CameraFront = DEFAULT_CAMERAFRONT;
		m_Yaw = DEFAULT_YAW;
		m_Pitch = DEFAULT_PITCH;
		s_FOV = DEFAULT_FOV;
		m_bMotionOn = false;
		m_ModelMotionRotation = 0.f;
	}

}
