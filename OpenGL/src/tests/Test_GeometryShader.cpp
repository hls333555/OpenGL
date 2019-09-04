#include "Test_GeometryShader.h"

#include "Renderer.h"
#include "imgui/imgui.h"

#include "VertexBufferLayout.h"

namespace test
{
	Test_GeometryShader::Test_GeometryShader()
		: m_Proj(glm::ortho(0.f, WINDOW_WIDTH, 0.f, WINDOW_HEIGHT, -1.f, 1.f))
		, m_View(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.f)))
	{
		float positions[] = {
			// Position   // Colors
			-0.5f,  0.5f, 0.5f, 0.f,  0.f,  // 0
			-0.5f, -0.5f, 0.f,  0.5f, 0.f,  // 1
			 0.5f, -0.5f, 0.f,  0.f,  0.5f, // 2
			 0.5f,  0.5f, 0.5f, 0.5f, 0.5f  // 3
		};

		unsigned int indices[] = {
			0, 1, 2, 3
		};

		m_VAO.reset(new VertexArray());
		m_VBO.reset(new VertexBuffer(positions, 20 * sizeof(float)));
		VertexBufferLayout layout;
		layout.Push<float>(2);
		layout.Push<float>(3);
		m_VAO->AddBuffer(*m_VBO, layout);
		m_IBO.reset(new IndexBuffer(indices, 4));
		m_Shader.reset(new Shader("res/shaders/BasicGS.shader"));

	}

	void Test_GeometryShader::OnRender()
	{
		Renderer renderer;
		{
			renderer.Draw(*m_VAO, *m_IBO, *m_Shader, DrawMode::POINTS);
		}
	}

	void Test_GeometryShader::OnImGuiRender()
	{

	}
}
