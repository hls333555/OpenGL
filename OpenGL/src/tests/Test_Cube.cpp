#include "Test_Cube.h"

#include "VertexBufferLayout.h"

#include "imgui/imgui.h"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

// Put it at last!
#include <GLFW/glfw3.h>

namespace test
{
	Test_Cube::Test_Cube()
		: m_Proj(glm::perspective(glm::radians(45.f), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f))
		, m_CameraRotRadius(1.5f)
		, m_DeltaX(glm::radians(45.f)), m_DeltaY(1)
		, m_CameraYawRotSpeed(1.f), m_CameraPitchRotSpeed(1.f)
		, m_PitchMax(1.f), m_PitchMin(-1.f)
		, m_ModelRotSpeed(90.f)
	{
		ResetView();

		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it is closer to the camera than the former one
		glDepthFunc(GL_LESS);

		GLCALL(glEnable(GL_BLEND));
		// Set this to blend transparency properly
		GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

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

		m_Texture.reset(new Texture("res/textures/Logo_Trans.png"));
		m_Texture->Bind();
		m_Shader->SetUniform1i("u_Texture", 0);

	}

	void Test_Cube::OnUpdate(float deltaTime)
	{
		static bool bFirstPress = true;
		if (glfwGetMouseButton(Test::s_Window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
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
			m_DeltaX += (xpos - m_LastXPos) * m_CameraYawRotSpeed * deltaTime;
			m_DeltaY = glm::clamp(float(m_DeltaY + (ypos - m_LastYPos) * m_CameraPitchRotSpeed * deltaTime), m_PitchMin, m_PitchMax);
			m_LastXPos = xpos;
			m_LastYPos = ypos;
			
		}
		else
		{
			bFirstPress = true;
			// When right mouse button is rleased, restore cursor behavior
			glfwSetInputMode(Test::s_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

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
			cameraPos = glm::vec3(m_CameraRotRadius * glm::cos(m_DeltaX), m_DeltaY, m_CameraRotRadius * glm::sin(m_DeltaX));
			m_View = glm::lookAt(cameraPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			m_Shader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
		}

		renderer.Draw(*m_VAO, *m_IBO, *m_Shader);

	}

	void Test_Cube::OnImGuiRender()
	{
		ImGui::Text("Hold right mouse button and move to rotate the camera!");
		ImGui::Checkbox("Motion", &bMotionOn);
		ImGui::SliderFloat("Motion Speed", &m_ModelRotSpeed, 10.f, 360.f);
		if (ImGui::Button("Reset View"))
		{
			ResetView();
		}
	}

	void Test_Cube::ResetView()
	{
		bMotionOn = false;
		m_DeltaX = glm::radians(45.f);
		m_DeltaY = 1;
		m_ModelRotation = 0.f;
	}

}
