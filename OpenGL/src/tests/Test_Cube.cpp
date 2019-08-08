#include "Test_Cube.h"

#include "VertexBufferLayout.h"

#include "glm/gtc/matrix_transform.hpp"

namespace test
{
	Test_Cube::Test_Cube()
		: m_Proj(glm::perspective(glm::radians(45.f), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.f))
		, m_View(glm::lookAt(glm::vec3(1, 1, -1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)))
	{
		float vertices[] = {
			// ---Begin: Top---
			0.f,  0.5f, -0.5f, // 0
			0.f,  0.5f,  0.f,  // 1
			0.5f, 0.5f,  0.f,  // 2
			0.5f, 0.5f, -0.5f, // 3
			// ---Begin: Front---
			0.f,  0.f,  -0.5f, // 4
			0.f,  0.5f, -0.5f, // 5
			0.5f, 0.5f, -0.5f, // 6
			0.5f, 0.f,  -0.5f, // 7
			// ---Begin: Left---
			0.f, 0.f,  -0.5f,  // 8
			0.f, 0.5f, -0.5f,  // 9
			0.f, 0.5f,  0.f,   // 10
			0.f, 0.f,   0.f,   // 11
			// ---Begin: Back---
			0.f,  0.f,  0.f,   // 12
			0.f,  0.5f, 0.f,   // 13
			0.5f, 0.5f, 0.f,   // 14
			0.5f, 0.f,  0.f,   // 15
			// ---Begin: Right---
			0.5f, 0.f,  -0.5f, // 16 
			0.5f, 0.5f, -0.5f, // 17
			0.5f, 0.5f,  0.f,  // 18
			0.5f, 0.f,   0.f,  // 19
			// ---Begin: Bottom---
			0.f,  0.f, -0.5f,  // 20
			0.f,  0.f,  0.f,   // 21
			0.5f, 0.f,  0.f,   // 22
			0.5f, 0.f, -0.5f   // 23
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

		m_VBO.reset(new VertexBuffer(vertices, 72 * sizeof(float)));

		VertexBufferLayout layout;
		layout.Push<float>(3);
		m_VAO->AddBuffer(*m_VBO, layout);

		m_IBO.reset(new IndexBuffer(indices, 36));

		m_Shader.reset(new Shader("res/shaders/Cube.shader"));
		m_Shader->Bind();

	}

	void Test_Cube::OnRender()
	{
		Renderer renderer;
		// Move cube to (0, 0, 0)
		glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.25f, 0.25f));
		m_Shader->SetUniformMat4f("u_Model", model);
		m_Shader->SetUniformMat4f("u_ViewProjection", m_Proj * m_View);
		renderer.Draw(*m_VAO, *m_IBO, *m_Shader);

	}

	void Test_Cube::OnImGuiRender()
	{

	}

}
