#include "Test_Instancing.h"

#include "Renderer.h"
#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#include "VertexBufferLayout.h"
#include "Mesh.h"

// Put it at last!
#include <GLFW/glfw3.h>

namespace test
{
	Test_BasicInstancing::Test_BasicInstancing()
	{
		// The prototype square is at the top-left, NDC is used in this class
		float positions[] = {
			// Positions   // Colors
			-0.975f,  0.825f, 0.5f, 0.f,  0.f,  // 0
			-0.825f,  0.825f, 0.f,  0.5f, 0.f,  // 1
			-0.825f,  0.975f, 0.f,  0.f,  0.5f, // 2
			-0.975f,  0.975f, 0.5f, 0.5f, 0.5f  // 3
		};

		unsigned int indices[] = {
			0, 1, 3,
			3, 1, 2
		};

		glm::vec2 translations[100];
		int index = 0;
		float offset = 0.2f;
		for (int y = 0; y < 10; ++y)
		{
			for (int x = 0; x < 10; ++x)
			{
				glm::vec2 trans;
				trans.x = x * offset;
				trans.y = -y * offset;
				translations[index++] = trans;
			}
		}

		m_VAO.reset(new VertexArray());
		m_VBO.reset(new VertexBuffer(positions, 20 * sizeof(float)));
		VertexBufferLayout layout;
		layout.Push<float>(2);
		layout.Push<float>(3);
		m_VAO->AddBuffer(*m_VBO, layout);
		m_IBO.reset(new IndexBuffer(indices, 6));
		m_Shader.reset(new Shader("res/shaders/BasicColor_Instanced.shader"));

		// TODO: needs abstraction for instancing
		m_TranslationsVBO.reset(new VertexBuffer(translations, 100 * sizeof(glm::vec2)));
		m_TranslationsVBO->Bind();
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		// Update the content of the vertex attribute per instace
		glVertexAttribDivisor(2, 1);

	}

	void Test_BasicInstancing::OnRender()
	{
		Renderer renderer;
		{
			// TODO: needs abstraction for instancing
			m_Shader->Bind();
			m_VAO->Bind();
			m_IBO->Bind();
			GLCALL(glDrawElementsInstanced(GL_TRIANGLES, m_IBO->GetCount(), GL_UNSIGNED_INT, nullptr, 100));
		}
	}

#define DEFAULT_CAMERAFRONT glm::vec3(0.f, 0.f, -1.f)
#define DEFAULT_CAMERAPOS	DEFAULT_CAMERAFRONT * -m_CameraOrbitRadius
#define DEFAULT_YAW			-90.f
#define DEFAULT_PITCH		0.f
#define DEFAULT_FOV			45.f

	float Test_AdvancedInstancing::s_FOV = DEFAULT_FOV;
	float Test_AdvancedInstancing::s_FOVMin = 1.f;
	float Test_AdvancedInstancing::s_FOVMax = 90.f;

	Test_AdvancedInstancing::Test_AdvancedInstancing()
		: m_PlanetRotSpeed(90.f)
		, m_RockOrbitRadius(70.f)
		, m_RockOrbitSpeed(0.1f)
		, m_RandomOffsetBound(10.f)
		, m_CameraOrbitRadius(50.f)
		, m_CameraPos(DEFAULT_CAMERAPOS)
		, m_CameraFront(DEFAULT_CAMERAFRONT)
		, m_CameraUp(glm::vec3(0.f, 1.f, 0.f))
		, m_CameraMoveSpeed(10.f)
		, m_CameraRotSpeed(10.f)
		, m_Yaw(DEFAULT_YAW), m_Pitch(DEFAULT_PITCH)
	{
		// WORKAROUND: C style function pointer must take a static function if it is a member function!
		glfwSetScrollCallback(Test::s_Window, OnMouseScroll);

		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it is closer to the camera than the former one
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);

		m_PlanetModel.reset(new Model("res/meshes/Planet/planet.obj"));
		m_PlanetShader.reset(new Shader("res/shaders/BasicModel.shader"));

		m_RockModel.reset(new Model("res/meshes/Rock/rock.obj"));
		m_RockShader.reset(new Shader("res/shaders/Model_Instanced.shader"));

		srand((unsigned int)glfwGetTime());
		// Initialize random positions of rocks
		for (unsigned int i = 0; i < ROCK_AMOUNT; ++i)
		{
			// Angle position per rock instance
			m_RockTransforms[i].Angle = 360.f / ROCK_AMOUNT * i;
			// Random offset in [-m_RandomOffsetBound, m_RandomOffsetBound] applied to rocks in xz plane
			m_RockTransforms[i].offsetX = rand() % (int)(m_RandomOffsetBound * 2 * 100 + 1) / 100.f - m_RandomOffsetBound;
			float x = m_RockOrbitRadius * cos(m_RockTransforms[i].Angle) + m_RockTransforms[i].offsetX;
			// Keep height of rocks smaller compared to x and z
			float y = m_RockTransforms[i].offsetX * 0.4f;
			// Re-randomize offset for z axis
			m_RockTransforms[i].offsetZ = rand() % (int)(m_RandomOffsetBound * 2 * 100 + 1) / 100.f - m_RandomOffsetBound;
			float z = m_RockOrbitRadius * sin(m_RockTransforms[i].Angle) + m_RockTransforms[i].offsetZ;
			// Set translation matrix
			glm::mat4 trans = glm::translate(glm::mat4(1.f), glm::vec3(x, y, z));

			// Set rotation matrix
			m_RockTransforms[i].rot = glm::rotate(glm::mat4(1.f), glm::radians((float)(rand() % 360)), glm::vec3(0.4f, 0.6f, 0.8f));

			// Random scale in [0.05, 0.25]
			float scaleVal = rand() % 21 / 100.f + 0.05f;
			// Set scale matrix
			m_RockTransforms[i].scale = glm::scale(glm::mat4(1.f), glm::vec3(scaleVal));

			// Combine into model matrix
			m_RockMatrices[i] = trans * m_RockTransforms[i].rot * m_RockTransforms[i].scale;
		}

		// TODO: needs abstraction for instancing
		m_RockMatricesVBO.reset(new VertexBuffer(m_RockMatrices, ROCK_AMOUNT * sizeof(glm::mat4)));
		m_RockMatricesVBO->Bind();
		for (const auto& mesh : m_RockModel->GetMeshes())
		{
			auto& VAO = mesh->GetVAO();
			VAO->Bind();
			// Since the max amount of data allowed as a vertex attribute is equal to a vec4 and mat4 is 4 vec4s
			// we have to reserve 4 vertex attributes for the matrix
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
			glVertexAttribDivisor(3, 1);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)sizeof(glm::vec4));
			glVertexAttribDivisor(4, 1);
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
			glVertexAttribDivisor(5, 1);
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
			glVertexAttribDivisor(6, 1);
		}

	}

	void Test_AdvancedInstancing::OnUpdate(float deltaTime)
	{
		ProcessInput(Test::s_Window, deltaTime);

		Renderer renderer;
		m_Proj = glm::perspective(glm::radians(s_FOV), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.f);
		m_View = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
		if (m_bMotionOn)
		{
			m_PlanetMotionRotation += m_PlanetRotSpeed * deltaTime;
			m_PlanetMotionRotation = m_PlanetMotionRotation > 360.f ? 0.f : m_PlanetMotionRotation;
		}
		
		// Render center planet
		{
			glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -2.f, 0.f)) *
				// Motion rotation
				glm::rotate(glm::mat4(1.f), glm::radians(m_PlanetMotionRotation), glm::vec3(0.f, 1.f, 0.f)) *
				glm::scale(glm::mat4(1.f), glm::vec3(3.f));

			m_PlanetShader->Bind();
			m_PlanetShader->SetUniformMat4f("u_Model", model);
			m_PlanetShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);

			m_PlanetModel->Draw(*m_PlanetShader);
		}
		// Render rocks with instancing method
		{
			if (m_bMotionOn)
			{
				// TODO: update it with ROCK_AMOUNT being a quite large value every frame will cause a terrible framerate drop
				for (unsigned int i = 0; i < ROCK_AMOUNT; ++i)
				{
					m_RockTransforms[i].Angle += m_RockOrbitSpeed * deltaTime;
					float x = m_RockOrbitRadius * cos(m_RockTransforms[i].Angle) + m_RockTransforms[i].offsetX;
					// Keep height of rocks smaller compared to x and z
					float y = m_RockTransforms[i].offsetX * 0.4f;
					float z = m_RockOrbitRadius * sin(m_RockTransforms[i].Angle) + m_RockTransforms[i].offsetZ;
					// Set translation matrix
					glm::mat4 trans = glm::translate(glm::mat4(1.f), glm::vec3(x, y, z));

					// Combine into model matrix
					m_RockMatrices[i] = trans * m_RockTransforms[i].rot * m_RockTransforms[i].scale;
				}

				// TODO: needs abstraction for instancing
				m_RockMatricesVBO.reset(new VertexBuffer(m_RockMatrices, ROCK_AMOUNT * sizeof(glm::mat4)));
				m_RockMatricesVBO->Bind();
				for (const auto& mesh : m_RockModel->GetMeshes())
				{
					auto& VAO = mesh->GetVAO();
					VAO->Bind();
					// Since the max amount of data allowed as a vertex attribute is equal to a vec4 and mat4 is 4 vec4s
					// we have to reserve 4 vertex attributes for the matrix
					glEnableVertexAttribArray(3);
					glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
					glVertexAttribDivisor(3, 1);
					glEnableVertexAttribArray(4);
					glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)sizeof(glm::vec4));
					glVertexAttribDivisor(4, 1);
					glEnableVertexAttribArray(5);
					glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
					glVertexAttribDivisor(5, 1);
					glEnableVertexAttribArray(6);
					glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));
					glVertexAttribDivisor(6, 1);
				}
			}

			m_RockShader->Bind();
			m_RockShader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
			m_RockShader->SetUniform1i("u_Texture", 0);
			// TODO: needs abstraction for instancing
			for (const auto& mesh : m_RockModel->GetMeshes())
			{
				for (unsigned int i = 0; i < mesh->GetTextures().size(); ++i)
				{
					mesh->GetTextures()[i]->Bind(i);
				}
				mesh->GetVAO()->Bind();
				mesh->GetIBO()->Bind();

				GLCALL(glDrawElementsInstanced(GL_TRIANGLES, mesh->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr, ROCK_AMOUNT));
			}
		}

	}

	void Test_AdvancedInstancing::OnImGuiRender()
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
			ImGui::Checkbox(u8"观赏模式", &m_bMotionOn);
			ImGui::SliderFloat(u8"行星运动速度", &m_PlanetRotSpeed, 10.f, 360.f);
			ImGui::SliderFloat(u8"陨石运动速度", &m_RockOrbitSpeed, 0.01f, 1.f);

		}

	}

	void Test_AdvancedInstancing::ProcessInput(GLFWwindow* window, float deltaTime)
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

	void Test_AdvancedInstancing::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		s_FOV = glm::clamp(float(s_FOV - yoffset), s_FOVMin, s_FOVMax);
	}

	void Test_AdvancedInstancing::ResetView()
	{
		m_bMotionOn = false;
		m_PlanetMotionRotation = 0.f;

		m_CameraPos = DEFAULT_CAMERAPOS;
		m_CameraFront = DEFAULT_CAMERAFRONT;
		m_Yaw = DEFAULT_YAW;
		m_Pitch = DEFAULT_PITCH;
		s_FOV = DEFAULT_FOV;
	}

}
